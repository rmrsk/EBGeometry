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
base: a5b6568
base-fingerprint: dbe9a8dcc095d10be7b06a98ea17abbc361cf01d
head: 382e920
head-fingerprint: 1a1304a7e01f1c6dde66162828773f28c10c5885
cases: 264
rounds: 2
reps: 3
date: 2026-11-11
-->

## Host comparison: yellow

**Accepted:** We accept it as a side-effect of the algorithmic work.

`382e920` against `a5b6568` (origin/main), 2026-11-11.

**VERDICT: 1 minor regression; look at it before accepting (see Flagged above)**

Performance comparison: 0 major, 1 minor, 4 faster, 0 differ, 0 lost, 264 cases

- base: a5b6568 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 382e920 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 332 ms | 332 ms | +0.1% | [+0.1%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 177 ms | 178 ms | +0.5% | [+0.4%, +0.6%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 513 ms | 514 ms | +0.3% | [+0.0%, +0.7%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 347 ms | 347 ms | +0.0% | [+0.0%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 186 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 563 ms | 563 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 41.9 ms | 41.9 ms | +0.0% | [-1.7%, +1.7%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 75.6 ms | 76.0 ms | +0.5% | [+0.1%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.9 ms | 55.8 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.3 ms | 51.4 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.4 ms | 64.6 ms | +0.3% | [+0.2%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.8 ms | 56.0 ms | +2.2% | [+0.0%, +4.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.6 ms | 60.5 ms | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.3 ms | 72.2 ms | -0.0% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 71.8 ms | 72.0 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.5 ms | 77.5 ms | +0.0% | [-0.7%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 64.3 ms | 64.3 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 99.6 ms | 100.0 ms | +0.4% | [-0.7%, +1.5%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 76.8 ms | 76.9 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 109 ms | 109 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | 63.8 ms | 64.0 ms | +0.3% | [+0.0%, +0.6%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | 95.7 ms | 95.6 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | 76.0 ms | 75.9 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | 101 ms | 101 ms | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.3 ms | 19.5 ms | +0.7% | [+0.1%, +1.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 41.0 ms | 41.2 ms | +0.5% | [+0.4%, +0.6%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 72.9 ms | 73.0 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.2 ms | 20.2 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 43.5 ms | 43.5 ms | -0.1% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 81.1 ms | 81.0 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | 140 ms | 141 ms | +0.6% | [-0.9%, +2.2%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | 76.8 ms | 76.8 ms | +0.0% | [-2.2%, +2.4%] | +0.0% | ~ | surface/leaf 1.84x |
| `ClosestSurface/union/closestSurface/float/host` | 146 ms | 146 ms | -0.4% | [-0.8%, -0.0%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/float/host` | 76.3 ms | 76.5 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ | surface/leaf 1.90x |
| `ClosestSurface/cloud/closestSurface/double/host` | 156 ms | 156 ms | -0.2% | [-0.3%, -0.2%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | 80.4 ms | 80.0 ms | -0.6% | [-0.6%, -0.5%] | +0.0% | ~ | surface/leaf 1.95x |
| `ClosestSurface/union/closestSurface/double/host` | 153 ms | 153 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/double/host` | 81.2 ms | 81.1 ms | -0.1% | [-0.6%, +0.3%] | +0.0% | ~ | surface/leaf 1.89x |
| `Fused/csg/signedDistance/float/host` | 4.49 ms | 4.46 ms | -0.5% | [-1.5%, +0.5%] | - | ~ | tape/fused 2.96x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | tape/fused 2.04x |
| `Fused/chain16/signedDistance/float/host` | 980 µs | 980 µs | -0.1% | [-2.2%, +2.1%] | - | ~ | tape/fused 16.57x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.1 ms | 46.0 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.3 ms | 46.3 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.60 ms | 4.62 ms | +0.6% | [-0.3%, +1.4%] | - | ~ | tape/fused 2.83x |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.8 ms | +0.0% | [-0.5%, +0.6%] | - | ~ | tape/fused 2.06x |
| `Fused/chain16/signedDistance/double/host` | 3.89 ms | 3.93 ms | +1.2% | [-0.2%, +2.6%] | - | ~ | tape/fused 4.17x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.8 ms | 52.8 ms | -0.1% | [-0.8%, +0.7%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.3 ms | 53.1 ms | -0.3% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 82.2 ms | 81.9 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 229 ms | 229 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 113 ms | -0.2% | [-0.7%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 261 ms | 261 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.2 ms | 69.9 ms | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 213 ms | 214 ms | +0.3% | [+0.0%, +0.5%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.7 ms | 97.6 ms | +1.0% | [+0.4%, +1.6%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 30.2 ms | 30.0 ms | -0.7% | [-2.2%, +0.7%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 766 ms | 757 ms | -1.1% | [-2.5%, +0.3%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 117 ms | 118 ms | +0.4% | [+0.4%, +0.5%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 43.3 ms | 43.7 ms | +0.9% | [-1.9%, +3.7%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 859 ms | 854 ms | -0.5% | [-2.3%, +1.2%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.6 ms | 26.5 ms | -0.3% | [-0.8%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 26.8 ms | 27.2 ms | +1.6% | [+0.9%, +2.4%] | +0.0% | ~ | checksum differs (13249.91874 -> 13258.03817) |
| `MeshCSG/smooth-tape/firstHit/float/host` | 318 ms | 318 ms | +0.1% | [+0.0%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 239 ms | 239 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ | checksum differs (13363.01869 -> 13367.93495) |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.3 ms | 90.4 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | tape/fused 3.52x |
| `MeshCSG/smooth-fused/frame/float/host` | 99.8 ms | 99.7 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ | tape/fused 2.39x; checksum differs (13364.92653 -> 13369.81935) |
| `MeshCSG/union-tape/firstHit/double/host` | 29.7 ms | 29.3 ms | -1.2% | [-1.4%, -1.0%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.6 ms | 30.1 ms | -1.7% | [-4.0%, +0.7%] | +0.0% | ~ | checksum differs (13249.88684 -> 13258.0063) |
| `MeshCSG/smooth-tape/firstHit/double/host` | 357 ms | 358 ms | +0.3% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 254 ms | 255 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ | checksum differs (13363.01454 -> 13367.9308) |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.0 ms | 96.8 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ | tape/fused 3.70x |
| `MeshCSG/smooth-fused/frame/double/host` | 107 ms | 107 ms | +0.4% | [-0.4%, +1.2%] | +0.0% | ~ | tape/fused 2.38x; checksum differs (13364.92631 -> 13369.81911) |
| `MeshReconcile/armadillo/reconcile/float/host` | 131 ms | 131 ms | +0.4% | [+0.2%, +0.5%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 41.5 ms | 41.4 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ | reconcile/scheduled 3.16x |
| `MeshReconcile/armadillo/reconcile/double/host` | 154 ms | 156 ms | +1.0% | [+0.3%, +1.6%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 56.8 ms | 56.4 ms | -0.7% | [-1.0%, -0.4%] | +0.0% | ~ | reconcile/scheduled 2.76x |
| `PointCloudBVH/uniform/build/float/host` | 53.6 ms | 53.3 ms | -0.7% | [-0.8%, -0.6%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 144 ms | 143 ms | -0.6% | [-0.8%, -0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 352 ms | 354 ms | +0.5% | [+0.1%, +0.9%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 85.1 ms | 79.3 ms | -6.9% | [-13.2%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.70 ms | 6.69 ms | -0.2% | [-1.2%, +0.8%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 55.7 ms | 55.9 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 156 ms | 157 ms | +0.6% | [+0.2%, +1.0%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 385 ms | 384 ms | -0.1% | [-0.4%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 87.0 ms | 86.8 ms | -0.2% | [-0.9%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.87 ms | 6.91 ms | +0.6% | [+0.1%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 13.9 ms | 13.6 ms | -1.7% | [-5.0%, +1.9%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 118 ms | 118 ms | -0.2% | [-0.9%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 473 ms | 476 ms | +0.8% | [+0.6%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 143 ms | 142 ms | -0.4% | [-0.5%, -0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 804 ms | 800 ms | -0.5% | [-0.9%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 14.9 ms | 15.1 ms | +1.4% | [-0.1%, +2.9%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 120 ms | 120 ms | -0.1% | [-0.3%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 492 ms | 492 ms | +0.1% | [-0.1%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 144 ms | 144 ms | -0.3% | [-0.6%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 800 ms | 789 ms | -1.4% | [-1.8%, -1.0%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 54.4 ms | 54.7 ms | +0.6% | [+0.4%, +0.7%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 128 ms | 128 ms | -0.2% | [-0.5%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 260 ms | 260 ms | -0.2% | [-0.8%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 147 ms | 148 ms | +0.4% | [+0.3%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 134 ms | 133 ms | -0.0% | [-0.3%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 57.4 ms | 57.3 ms | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 139 ms | 139 ms | -0.2% | [-0.8%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 291 ms | 291 ms | +0.1% | [-0.1%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 161 ms | 161 ms | -0.3% | [-0.8%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 147 ms | 148 ms | +0.3% | [-0.1%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 15.2 ms | 11.7 ms | -22.9% | [-37.3%, -0.1%] | +0.0% | faster |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 115 ms | 115 ms | -0.8% | [-0.9%, -0.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 318 ms | 317 ms | -0.2% | [-0.6%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 155 ms | 155 ms | -0.5% | [-0.6%, -0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 321 ms | 320 ms | -0.2% | [-0.3%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.9 ms | 13.5 ms | -2.4% | [-4.2%, -0.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 117 ms | 115 ms | -1.2% | [-1.5%, -0.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 348 ms | 347 ms | -0.5% | [-0.7%, -0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 165 ms | 165 ms | +0.1% | [-0.2%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 309 ms | 309 ms | -0.1% | [-0.4%, +0.2%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 35.2 ms | 35.1 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 38.1 ms | 38.1 ms | -0.0% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | 8.32 ms | 8.68 ms | +4.3% | [-0.9%, +9.6%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | 9.60 ms | 9.50 ms | -0.9% | [-4.1%, +2.3%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.4 ms | 46.4 ms | +0.2% | [-0.3%, +0.7%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.6 ms | 45.6 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.5 ms | 40.5 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.4 ms | 78.8 ms | +0.6% | [+0.2%, +0.9%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.9 ms | 65.4 ms | -0.7% | [-1.0%, -0.4%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.22 ms | 5.20 ms | -0.3% | [-0.5%, -0.2%] | - | ~ | tape/fused 1.48x |
| `Ray/csg-tape/firstHit/float/host` | 7.61 ms | 7.69 ms | +1.0% | [-0.1%, +2.2%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 52.5 ms | 52.4 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/double/host` | 50.9 ms | 51.2 ms | +0.5% | [-0.0%, +1.1%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.7 ms | 44.9 ms | +0.5% | [+0.2%, +0.8%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 87.6 ms | 87.5 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 74.5 ms | 73.7 ms | -1.1% | [-1.2%, -1.0%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.52 ms | 5.49 ms | -0.5% | [-0.6%, -0.5%] | - | ~ | tape/fused 1.43x |
| `Ray/csg-tape/firstHit/double/host` | 7.87 ms | 7.84 ms | -0.4% | [-0.7%, -0.0%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.6 ms | 17.5 ms | -5.8% | [-6.2%, -5.4%] | - | faster | marched/traced 6.87x (was 6.44x) |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 120 ms | 120 ms | +0.5% | [-0.6%, +1.6%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.1 ms | 22.2 ms | +0.8% | [-0.5%, +2.1%] | +0.0% | ~ | marched/traced 6.53x (was 6.79x) |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 150 ms | 145 ms | -3.1% | [-3.5%, -2.7%] | +0.0% | faster |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.5 ms | 35.5 ms | -0.0% | [-0.9%, +0.8%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 18.8 ms | 19.2 ms | +1.8% | [+0.3%, +3.3%] | - | ~ | marched/traced 6.64x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 127 ms | 127 ms | +0.4% | [-0.3%, +1.1%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.6 ms | 24.1 ms | +2.2% | [+0.3%, +4.0%] | +0.0% | ~ | marched/traced 6.27x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 150 ms | 151 ms | +0.9% | [+0.0%, +1.7%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 38.0 ms | 37.9 ms | -0.1% | [-0.9%, +0.6%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.5 ms | 46.4 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.3 ms | 46.4 ms | +0.2% | [-0.0%, +0.3%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.63 ms | 7.76 ms | +1.7% | [+0.3%, +3.0%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.14 ms | 6.20 ms | +0.9% | [-1.4%, +3.2%] | +0.0% | ~ | hit/crossing 1.25x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 177 ms | 177 ms | +0.4% | [+0.3%, +0.4%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 157 ms | 157 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.5 ms | 68.7 ms | +0.3% | [-0.0%, +0.7%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.5 ms | 56.3 ms | -0.3% | [-0.7%, +0.0%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.2 ms | 52.3 ms | +0.2% | [-0.8%, +1.2%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.3 ms | 52.3 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 8.00 ms | 7.96 ms | -0.5% | [-0.8%, -0.1%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.37 ms | 6.22 ms | -2.3% | [-4.9%, +0.4%] | +0.0% | ~ | hit/crossing 1.28x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 201 ms | 202 ms | +0.5% | [-0.3%, +1.3%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 178 ms | 180 ms | +1.4% | [+0.2%, +2.6%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 77.4 ms | 77.5 ms | +0.1% | [-0.0%, +0.3%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 64.2 ms | 64.2 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ | hit/crossing 1.21x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.0 ms | 11.1 ms | +0.5% | [-0.2%, +1.2%] | +0.0% | ~ | marched/traced 4.66x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.7 ms | 51.6 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.12 ms | 8.00 ms | -1.5% | [-3.2%, +0.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.93 ms | 1.94 ms | +0.5% | [-1.0%, +2.0%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.91 ms | 6.80 ms | -1.6% | [-1.8%, -1.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.7 ms | 11.8 ms | +1.1% | [+0.4%, +1.8%] | +0.0% | ~ | marched/traced 5.14x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 60.3 ms | 60.7 ms | +0.7% | [+0.0%, +1.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.85 ms | 8.63 ms | -2.5% | [-5.8%, +0.9%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.93 ms | 1.92 ms | -0.5% | [-1.1%, +0.2%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.19 ms | 7.21 ms | +0.2% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 238 ms | 239 ms | +0.5% | [+0.4%, +0.7%] | +0.0% | ~ | marched/traced 1.37x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 327 ms | 328 ms | +0.4% | [+0.3%, +0.5%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 236 ms | 238 ms | +0.9% | [+0.7%, +1.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 255 ms | 254 ms | -0.5% | [-1.0%, -0.0%] | +0.0% | ~ | marched/traced 1.45x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 368 ms | 368 ms | -0.0% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 253 ms | 252 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | - | - | - | - | - | new |  |
| `Regions/particles/regionDistances/float/host` | - | - | - | - | - | new |  |
| `Regions/particles/regionByRegion/float/host` | - | - | - | - | - | new | each/one-pass 0.05x |
| `Regions/particles/domainLabels/float/host` | - | - | - | - | - | new |  |
| `Regions/assembly/firstRegionChange/float/host` | - | - | - | - | - | new |  |
| `Regions/assembly/firstHit/float/host` | - | - | - | - | - | new | regions/hit 1.35x |
| `Regions/particles/region/double/host` | - | - | - | - | - | new |  |
| `Regions/particles/regionDistances/double/host` | - | - | - | - | - | new |  |
| `Regions/particles/regionByRegion/double/host` | - | - | - | - | - | new | each/one-pass 0.05x |
| `Regions/particles/domainLabels/double/host` | - | - | - | - | - | new |  |
| `Regions/assembly/firstRegionChange/double/host` | - | - | - | - | - | new |  |
| `Regions/assembly/firstHit/double/host` | - | - | - | - | - | new | regions/hit 1.35x |
| `Render/csg/frame/float/host` | 19.0 ms | 19.4 ms | +1.9% | [+0.9%, +3.0%] | +0.0% | ~ | checksum differs (27647.32233 -> 27718.0892) |
| `Render/csg/frame-orthographic/float/host` | 18.9 ms | 19.4 ms | +2.7% | [+1.7%, +3.8%] | +0.0% | ~ | checksum differs (27353.23894 -> 27409.89965) |
| `Render/mesh/frame/float/host` | 18.7 ms | 18.4 ms | -1.6% | [-3.7%, +0.4%] | +0.0% | ~ | checksum differs (30123.03449 -> 30139.52904) |
| `Render/csg/frame/double/host` | 21.8 ms | 21.2 ms | -2.6% | [-3.5%, -1.7%] | +0.0% | ~ | checksum differs (27647.31803 -> 27718.0849) |
| `Render/csg/frame-orthographic/double/host` | 21.7 ms | 21.5 ms | -1.0% | [-2.0%, -0.0%] | +0.0% | ~ | checksum differs (27353.23854 -> 27409.89926) |
| `Render/mesh/frame/double/host` | 20.7 ms | 20.6 ms | -0.4% | [-1.5%, +0.7%] | +0.0% | ~ | checksum differs (30123.17077 -> 30139.62606) |
| `Shapes/ellipsoid/signedDistance/float/host` | 33.0 ms | 33.0 ms | -0.2% | [-0.4%, +0.0%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/float/host` | 5.00 ms | 4.97 ms | -0.7% | [-0.9%, -0.5%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/float/host` | 4.19 ms | 4.17 ms | -0.3% | [-0.5%, -0.2%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/float/host` | 17.3 ms | 17.3 ms | +0.4% | [-0.2%, +0.9%] | - | ~ |  |
| `Shapes/tube/signedDistance/float/host` | 4.32 ms | 4.35 ms | +0.6% | [-0.5%, +1.7%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/float/host` | 4.14 ms | 4.17 ms | +0.6% | [-1.1%, +2.3%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/float/host` | 2.22 ms | 2.23 ms | +0.5% | [-1.0%, +2.3%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/float/host` | 1.43 ms | 1.45 ms | +1.3% | [-0.4%, +3.0%] | - | ~ |  |
| `Shapes/wedge/signedDistance/float/host` | 2.07 ms | 2.05 ms | -1.0% | [-3.3%, +1.4%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/float/host` | 27.2 ms | 28.0 ms | +3.1% | [-0.6%, +6.8%] | - | ~ |  |
| `Shapes/ellipsoid/signedDistance/double/host` | 46.3 ms | 46.3 ms | +0.0% | [-0.6%, +0.6%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/double/host` | 5.37 ms | 5.47 ms | +2.0% | [+0.8%, +3.1%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/double/host` | 4.47 ms | 4.47 ms | +0.1% | [-0.1%, +0.3%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/double/host` | 22.4 ms | 22.6 ms | +0.8% | [-0.4%, +2.1%] | - | ~ |  |
| `Shapes/tube/signedDistance/double/host` | 4.56 ms | 4.54 ms | -0.4% | [-1.4%, +0.5%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/double/host` | 5.23 ms | 5.22 ms | -0.1% | [-1.1%, +1.0%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/double/host` | 2.50 ms | 2.48 ms | -0.7% | [-1.5%, +0.2%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/double/host` | 1.78 ms | 1.79 ms | +0.4% | [+0.0%, +0.7%] | - | ~ |  |
| `Shapes/wedge/signedDistance/double/host` | 2.09 ms | 2.05 ms | -1.9% | [-3.0%, -0.7%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/double/host` | 30.6 ms | 29.7 ms | -2.9% | [-4.5%, -1.3%] | - | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 56.6 ms | 56.8 ms | +0.2% | [-0.3%, +0.8%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 36.5 ms | 36.8 ms | +0.9% | [+0.6%, +1.1%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 92.2 ms | 92.1 ms | -0.1% | [-0.1%, -0.1%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 64.2 ms | 64.2 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 42.7 ms | 42.7 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 108 ms | 106 ms | -1.3% | [-2.1%, -0.4%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 74.9 ms | 74.8 ms | -0.2% | [-0.7%, +0.2%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 296 ms | 296 ms | -0.3% | [-0.5%, +0.0%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 296 ms | 297 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 87.2 ms | 86.8 ms | -0.5% | [-1.0%, +0.1%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 348 ms | 345 ms | -0.8% | [-2.3%, +0.6%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 346 ms | 346 ms | +0.1% | [-0.5%, +0.6%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 166 ms | 166 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 191 ms | 190 ms | -0.6% | [-0.8%, -0.4%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 145 ms | 145 ms | +0.0% | [-2.1%, +2.2%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 89.7 ms | 89.8 ms | +0.1% | [-0.4%, +0.5%] | +0.0% | ~ | surface/leaf 1.61x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 130 ms | 131 ms | +0.7% | [+0.5%, +0.9%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 72.3 ms | 73.0 ms | +1.0% | [+0.9%, +1.2%] | +0.0% | ~ | surface/leaf 1.79x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 61.3 ms | 61.6 ms | +0.5% | [+0.1%, +1.0%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 34.1 ms | 34.2 ms | +0.6% | [+0.4%, +0.7%] | +0.0% | ~ | surface/hit 1.80x |
| `SurfaceParts/boxes/closestSurface/double/host` | 144 ms | 146 ms | +1.2% | [+0.6%, +1.8%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 90.7 ms | 90.5 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ | surface/leaf 1.61x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 143 ms | 144 ms | +0.4% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 80.8 ms | 80.8 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | surface/leaf 1.78x |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 68.1 ms | 68.2 ms | +0.2% | [-0.4%, +0.8%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 38.9 ms | 38.5 ms | -1.0% | [-1.7%, -0.2%] | +0.0% | ~ | surface/hit 1.77x |
| `Tape/csg/signedDistance/float/host` | 13.0 ms | 13.2 ms | +1.9% | [+1.7%, +2.2%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.57 ms | 4.62 ms | +1.1% | [+0.0%, +2.2%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.1 ms | 25.1 ms | +0.1% | [-0.8%, +1.0%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 43.6 ms | 40.0 ms | -8.4% | [-15.8%, +0.4%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.74 ms | 1.77 ms | +1.7% | [+1.4%, +2.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 143 µs | 133 µs | -7.2% | [-8.0%, -6.4%] | - | faster |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.9 ms | 29.8 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.66 ms | 5.65 ms | -0.1% | [-1.0%, +0.8%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.3 ms | 16.2 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.65 ms | 4.67 ms | +0.3% | [+0.2%, +0.5%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.2 ms | 13.1 ms | -0.6% | [-0.9%, -0.4%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.66 ms | 4.58 ms | -1.7% | [-3.4%, +0.1%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.4 ms | 26.3 ms | -0.1% | [-1.4%, +1.3%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.0 ms | 41.4 ms | -1.3% | [-1.7%, -0.8%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.78 ms | 1.80 ms | +1.5% | [+0.0%, +3.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 293 µs | 280 µs | -4.2% | [-14.4%, +7.5%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.8 ms | 29.8 ms | -0.0% | [-0.7%, +0.7%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.71 ms | 7.74 ms | +0.4% | [-0.6%, +1.3%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.3 ms | 16.4 ms | +0.6% | [-0.2%, +1.3%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.81 ms | 7.79 ms | -0.3% | [-1.9%, +1.4%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 41.9 ms | 41.8 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 204 ms | 217 ms | +6.4% | [+6.2%, +6.7%] | +0.0% | minor |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 42.6 ms | 42.5 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 213 ms | 215 ms | +0.8% | [-0.0%, +1.6%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 546.5 s → 564.8 s (+3.3%), the largest peak 1326 MB → 1409 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.1 s | 9.1 s | -0.3% | 555 MB | 555 MB | +0.0% | ~ |
| `BVHRefit` | 3.9 s | 3.9 s | +0.9% | 469 MB | 481 MB | +2.6% | ~ |
| `BVHUnion` | 18.1 s | 17.8 s | -1.9% | 854 MB | 892 MB | +4.5% | ~ |
| `BVHUnionMixed` | 7.2 s | 7.3 s | +1.3% | 556 MB | 554 MB | -0.3% | ~ |
| `ClosestPrimitive` | 26.4 s | 27.8 s | +5.2% | 1211 MB | 1250 MB | +3.3% | ~ |
| `ClosestSurface` | 26.1 s | 26.9 s | +3.1% | 1153 MB | 1221 MB | +5.9% | ~ |
| `Fused` | 21.9 s | 23.4 s | +7.1% | 1051 MB | 1121 MB | +6.7% | ~ |
| `LaunchTuning` | 34.2 s | 35.0 s | +2.3% | 1192 MB | 1269 MB | +6.4% | ~ |
| `Mesh` | 14.6 s | 14.3 s | -1.7% | 775 MB | 784 MB | +1.2% | ~ |
| `MeshCSG` | 35.8 s | 36.8 s | +2.9% | 1228 MB | 1275 MB | +3.8% | ~ |
| `MeshReconcile` | 6.6 s | 6.3 s | -4.0% | 547 MB | 562 MB | +2.7% | ~ |
| `PointCloud` | 7.3 s | 7.1 s | -2.7% | 558 MB | 557 MB | -0.2% | ~ |
| `PointCloudBVHBuild` | 3.6 s | 3.6 s | -1.0% | 450 MB | 467 MB | +3.6% | ~ |
| `PointCloudHashGridBuild` | 3.0 s | 3.1 s | +1.4% | 402 MB | 450 MB | +11.8% | ~ |
| `Ray` | 33.2 s | 34.2 s | +3.0% | 1233 MB | 1310 MB | +6.2% | ~ |
| `RayCSG` | 30.3 s | 31.1 s | +2.6% | 1054 MB | 1114 MB | +5.8% | ~ |
| `RayCrossing` | 42.0 s | 43.3 s | +3.2% | 1232 MB | 1340 MB | +8.8% | ~ |
| `RayParticles` | 45.6 s | 47.0 s | +3.0% | 1294 MB | 1314 MB | +1.6% | ~ |
| `RayShell` | 35.2 s | 35.5 s | +0.7% | 1151 MB | 1242 MB | +7.9% | ~ |
| `Regions` | - | 28.2 s | | - | 1060 MB | | new |
| `Render` | 32.6 s | 34.4 s | +5.4% | 1143 MB | 1204 MB | +5.3% | ~ |
| `Shapes` | 9.2 s | 9.2 s | -0.3% | 552 MB | 553 MB | +0.3% | ~ |
| `SphereCloudBVH` | 4.7 s | 4.8 s | +2.3% | 492 MB | 506 MB | +2.9% | ~ |
| `SphereCloudRays` | 26.0 s | 27.6 s | +6.0% | 982 MB | 1057 MB | +7.7% | ~ |
| `SphereContacts` | 4.2 s | 4.1 s | -0.7% | 464 MB | 481 MB | +3.6% | ~ |
| `SurfaceParts` | 38.2 s | 40.4 s | +5.7% | 1326 MB | 1409 MB | +6.2% | ~ |
| `Tape` | 14.8 s | 16.5 s | +11.5% | 815 MB | 820 MB | +0.5% | ~ |
| `TapeBVHUnion` | 12.6 s | 14.3 s | +12.9% | 810 MB | 815 MB | +0.7% | ~ |
| **all, in both** | 546.5 s | 564.8 s | +3.3% | 1326 MB | 1409 MB | +6.2% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (2 cases): Shapes/cut-sphere/signedDistance/float/host, TapeBVHUnion/maps64-as-written/signedDistance/float/host

The probes differ between the sides (each side ran its own): Performance/Regions/main.cpp, Performance/SurfaceParts/main.cpp, Performance/Common/Inputs.hpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: a5b6568
base-fingerprint: dbe9a8dcc095d10be7b06a98ea17abbc361cf01d
head: 382e920
head-fingerprint: 1a1304a7e01f1c6dde66162828773f28c10c5885
cases: 362
rounds: 2
reps: 3
date: 2026-11-11
-->

## Device comparison (CUDA): yellow

**Accepted:** Accept as is.

`382e920` against `a5b6568` (origin/main), 2026-11-11.

**VERDICT: 1 minor regression; look at it before accepting (see Flagged above)**

Performance comparison: 0 major, 1 minor, 7 faster, 0 differ, 0 lost, 362 cases

- base: a5b6568 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 382e920 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.61 ms | 1.62 ms | +0.4% | [-0.2%, +0.9%] | - | ~ | device 22.5x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.41 ms | 3.44 ms | +0.9% | [-0.6%, +2.4%] | - | ~ | device 18.7x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.04 ms | 1.05 ms | +1.3% | [-0.2%, +2.9%] | - | ~ | device 19.4x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.32 ms | 2.30 ms | -0.5% | [-0.6%, -0.4%] | - | ~ | device 16.5x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.26 ms | 6.27 ms | +0.2% | [-2.6%, +3.2%] | - | ~ | device 7.1x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.9 ms | 11.9 ms | +0.5% | [+0.1%, +1.0%] | - | ~ | device 5.3x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.3 ms | 33.1 ms | -0.6% | [-0.8%, -0.4%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.0 ms | 57.0 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.3 ms | 20.5 ms | +1.1% | [-0.3%, +2.5%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.8 ms | 47.7 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.2 ms | 62.4 ms | +1.9% | [+1.0%, +2.9%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 109 ms | 111 ms | +1.9% | [+0.9%, +2.9%] | - | ~ | device 0.6x faster than the host |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.21 ms | 8.31 ms | +1.3% | [-2.7%, +5.3%] | +0.0% | ~ | device 4.8x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 9.88 ms | 9.76 ms | -1.2% | [-2.8%, +0.5%] | +0.0% | ~ | device 8.3x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 388 µs | 387 µs | -0.2% | [-0.7%, +0.2%] | - | ~ | device 15.3x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 846 µs | 845 µs | -0.1% | [-0.5%, +0.3%] | - | ~ | device 10.0x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.51 ms | 4.50 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.66 ms | 4.68 ms | +0.5% | [+0.1%, +0.8%] | - | ~ | device 1.8x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 586 µs | 586 µs | +0.1% | [-0.1%, +0.3%] | - | ~ | device 12.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.38 ms | 1.38 ms | -0.2% | [-0.6%, +0.1%] | - | ~ | device 8.2x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.79 ms | 6.76 ms | -0.3% | [-0.6%, +0.0%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.00 ms | 7.03 ms | +0.4% | [+0.3%, +0.6%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 908 µs | 907 µs | -0.1% | [-0.6%, +0.3%] | - | ~ | device 14.3x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.23 ms | 2.23 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 7.9x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.49 ms | 9.46 ms | -0.4% | [-0.7%, -0.0%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 1.8x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/float/hostdevice` | 851 µs | 850 µs | -0.0% | [-0.5%, +0.4%] | - | ~ | device 13.5x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/hostdevice` | 2.07 ms | 2.07 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 7.9x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/double/hostdevice` | 9.10 ms | 9.09 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 1.3x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/hostdevice` | 9.86 ms | 9.82 ms | -0.4% | [-0.6%, -0.1%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 314 µs | 314 µs | +0.1% | [-0.2%, +0.3%] | - | ~ | device 11.7x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.30 ms | 1.30 ms | -0.1% | [-0.1%, -0.1%] | - | ~ | device 6.2x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 58.8 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 4.0x faster than the host (was 3.3x) |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | -0.1% | [-0.1%, +0.0%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.39 ms | 5.39 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 521 ms | 516 ms | -0.9% | [-2.2%, +0.4%] | - | ~ | device 0.4x faster than the host |
| `ClosestSurface/cloud/closestSurface/float/hostdevice` | 20.2 ms | 20.1 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 4.1x faster than the host |
| `ClosestSurface/cloud/closestLeaf/float/hostdevice` | 8.34 ms | 8.34 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 4.7x faster than the host; surface/leaf 2.42x |
| `ClosestSurface/union/closestSurface/float/hostdevice` | 15.5 ms | 15.6 ms | +0.5% | [+0.4%, +0.6%] | - | ~ | device 8.8x faster than the host (was 9.6x) |
| `ClosestSurface/union/closestLeaf/float/hostdevice` | 6.18 ms | 6.19 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 11.3x faster than the host; surface/leaf 2.52x |
| `ClosestSurface/cloud/closestSurface/double/hostdevice` | 147 ms | 146 ms | -0.7% | [-1.9%, +0.5%] | - | ~ | device 0.5x faster than the host |
| `ClosestSurface/cloud/closestLeaf/double/hostdevice` | 60.0 ms | 60.0 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 0.7x faster than the host; surface/leaf 2.43x |
| `ClosestSurface/union/closestSurface/double/hostdevice` | 71.6 ms | 72.0 ms | +0.5% | [+0.0%, +0.9%] | - | ~ | device 1.8x faster than the host |
| `ClosestSurface/union/closestLeaf/double/hostdevice` | 28.0 ms | 28.0 ms | -0.1% | [-0.1%, +0.0%] | - | ~ | device 2.3x faster than the host; surface/leaf 2.57x |
| `Fused/csg/signedDistance/float/hostdevice` | 47.9 µs | 48.3 µs | +0.9% | [+0.1%, +1.8%] | - | ~ | device 23.6x faster than the host; tape/fused 4.51x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.9 µs | 49.9 µs | -0.1% | [-0.6%, +0.4%] | - | ~ | device 43.9x faster than the host; tape/fused 7.20x |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.9 µs | 47.6 µs | -0.6% | [-0.8%, -0.4%] | - | ~ | device 9.2x faster than the host; tape/fused 10.01x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.5 ms | 17.4 ms | -0.3% | [-1.2%, +0.6%] | - | ~ | device 6.7x faster than the host; tape/fused 1.79x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.1 ms | 31.2 ms | +0.1% | [+0.0%, +0.3%] | - | ~ | device 3.8x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | +0.0% | [+0.0%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 286 ms | 285 ms | -0.7% | [-1.4%, +0.1%] | - | ~ | device 0.4x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 287 ms | 286 ms | -0.4% | [-1.3%, +0.6%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.26 ms | 8.25 ms | -0.1% | [-1.4%, +1.1%] | +0.0% | ~ | device 10.7x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.26 ms | 8.17 ms | -1.1% | [-1.9%, -0.3%] | +0.0% | ~ | device 15.1x faster than the host (was 11.7x); default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.05 ms | 8.18 ms | +1.6% | [+0.3%, +2.8%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.27 ms | 8.22 ms | -0.7% | [-1.7%, +0.3%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 689 µs | 690 µs | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ | device 3.6x faster than the host (was 3.8x) |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.73 ms | 1.73 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ | device 1.5x faster than the host (was 1.5x); default/this 0.40x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.04 ms | 1.03 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ | device 2.4x faster than the host (was 2.5x); default/this 0.67x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 782 µs | 784 µs | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ | device 3.2x faster than the host; default/this 0.88x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 742 µs | 737 µs | -0.7% | [-1.0%, -0.4%] | +0.0% | ~ | device 12.0x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 823 µs | 817 µs | -0.8% | [-1.0%, -0.7%] | +0.0% | ~ | device 10.9x faster than the host; default/this 0.90x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 746 µs | 744 µs | -0.3% | [-1.8%, +1.3%] | +0.0% | ~ | device 12.0x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 735 µs | 738 µs | +0.4% | [-0.9%, +1.8%] | +0.0% | ~ | device 11.8x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 160 ms | 158 ms | -1.0% | [-2.5%, +0.5%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 158 ms | 158 ms | +0.1% | [-1.6%, +1.8%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 160 ms | 162 ms | +1.0% | [-2.5%, +4.5%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.98x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 158 ms | 161 ms | +2.0% | [+0.8%, +3.2%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.98x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.39 ms | 1.39 ms | +0.0% | [-0.0%, +0.1%] | +0.0% | ~ | device 1.9x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.78 ms | 2.79 ms | +0.6% | [+0.3%, +0.8%] | +0.0% | ~ | device 1.0x faster than the host; default/this 0.50x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.95 ms | 1.94 ms | -0.4% | [-0.8%, +0.0%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.72x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.56 ms | 1.56 ms | -0.1% | [-0.7%, +0.4%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.25 ms | 7.24 ms | -0.1% | [-0.3%, -0.0%] | +0.0% | ~ | device 1.2x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.28 ms | 7.25 ms | -0.5% | [-0.9%, -0.0%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.27 ms | 7.28 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.29 ms | 7.28 ms | -0.1% | [-0.6%, +0.5%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.33 ms | 8.34 ms | +0.1% | [-0.3%, +0.6%] | - | ~ | device 11.3x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.7 ms | 16.6 ms | -0.7% | [-1.5%, +0.1%] | - | ~ | device 8.2x faster than the host (was 7.7x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 164 ms | 164 ms | -0.5% | [-1.8%, +0.8%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 288 ms | 283 ms | -1.7% | [-2.6%, -0.9%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.5 ms | 24.7 ms | +0.9% | [-0.1%, +1.8%] | - | ~ | device 4.0x faster than the host (was 4.5x) |
| `MeshSDF/query-random/float/hostdevice` | 58.5 ms | 58.5 ms | +0.1% | [+0.1%, +0.1%] | - | ~ | device 2.5x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 158 ms | 159 ms | +0.4% | [-2.5%, +3.4%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 248 ms | 239 ms | -3.6% | [-4.7%, -2.5%] | - | faster | device 0.7x faster than the host (was 0.6x) |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.54 ms | 2.52 ms | -0.9% | [-1.6%, -0.3%] | - | ~ | device 4.2x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 4.30 ms | 4.26 ms | -0.9% | [-1.6%, -0.2%] | - | ~ | device 3.3x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 13.8 ms | 13.7 ms | -0.6% | [-0.7%, -0.5%] | - | ~ | device 4.0x faster than the host (was 4.5x); checksum differs (212007.2132 -> 212135.6679) |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 63.8 ms | 63.2 ms | -1.0% | [-1.7%, -0.3%] | - | ~ | device 4.2x faster than the host (was 3.2x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 93.8 ms | 93.1 ms | -0.7% | [-1.5%, +0.0%] | - | ~ | device 2.3x faster than the host (was 2.7x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 249 ms | 250 ms | +0.4% | [-1.5%, +2.3%] | - | ~ | device 3.3x faster than the host; checksum differs (213840.7542 -> 213917.8326) |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.6 ms | 24.7 ms | +0.6% | [-0.7%, +2.0%] | - | ~ | device 2.2x faster than the host (was 2.0x); tape/fused 2.55x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 27.0 ms | 26.7 ms | -1.0% | [-2.3%, +0.2%] | - | ~ | device 2.2x faster than the host (was 2.4x); tape/fused 3.48x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.0 ms | 71.0 ms | +0.0% | [-1.1%, +1.1%] | - | ~ | device 4.8x faster than the host (was 4.1x); tape/fused 3.52x; checksum differs (213874.5778 -> 213951.0518) |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.8 ms | 22.4 ms | -1.6% | [-4.7%, +1.8%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 44.0 ms | 44.3 ms | +0.8% | [-0.3%, +1.9%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 94.3 ms | 95.5 ms | +1.3% | [-2.4%, +5.0%] | - | ~ | device 0.6x faster than the host; checksum differs (212004.9163 -> 212133.3948) |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 530 ms | 538 ms | +1.7% | [-3.7%, +7.4%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 857 ms | 843 ms | -1.7% | [-3.6%, +0.3%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.04 s | 2.05 s | +0.8% | [-0.3%, +1.9%] | - | ~ | device 0.4x faster than the host (was 0.5x); checksum differs (213840.6225 -> 213917.701) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 191 ms | 200 ms | +4.9% | [-1.8%, +6.6%] | - | ~ | device 0.2x faster than the host (was 0.3x); tape/fused 2.69x (was 2.78x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 299 ms | 307 ms | +2.6% | [-1.5%, +7.0%] | - | ~ | device 0.2x faster than the host; tape/fused 2.75x (was 2.87x) |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 588 ms | 604 ms | +2.7% | [+0.0%, +5.3%] | - | ~ | device 0.5x faster than the host; tape/fused 3.40x; checksum differs (213874.3636 -> 213950.8768) |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.66 ms | 2.67 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ | device 15.6x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.33 ms | 5.33 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ | device 10.3x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 681 µs | 682 µs | +0.1% | [-0.4%, +0.5%] | - | ~ | device 11.0x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.84 ms | 1.83 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 995 µs | 998 µs | +0.3% | [-0.3%, +0.8%] | - | ~ | device 9.3x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.63 ms | 4.63 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 5.4x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 3.00 ms | 3.00 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.09 ms | 7.07 ms | -0.3% | [-0.6%, -0.1%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.82 ms | 7.78 ms | -0.4% | [-1.0%, +0.2%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.3 ms | 13.3 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | -0.1% | [-0.5%, +0.4%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.48 ms | 1.46 ms | -1.7% | [-4.6%, +1.2%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.90 ms | 3.89 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 527 µs | 523 µs | -0.8% | [-1.3%, -0.2%] | - | ~ | device 15.9x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.68 ms | 4.69 ms | +0.2% | [-0.3%, +0.6%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.36 ms | 4.34 ms | -0.4% | [-0.5%, -0.3%] | - | ~ | device 5.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.04 ms | 8.06 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.9 ms | 15.9 ms | +0.2% | [-0.4%, +0.9%] | - | ~ | device 0.9x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.01 ms | 4.01 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.6 ms | 17.6 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 15.1 ms | 15.1 ms | -0.2% | [-1.2%, +0.9%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.57 ms | 1.58 ms | +0.6% | [+0.5%, +0.6%] | - | ~ | device 10.4x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.60 ms | 4.59 ms | -0.1% | [-0.1%, +0.0%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 831 µs | 834 µs | +0.4% | [-0.2%, +0.9%] | - | ~ | device 10.3x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.88 ms | 2.88 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 6.9x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.18 ms | 4.20 ms | +0.4% | [-0.1%, +1.0%] | - | ~ | device 5.4x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.6 ms | 16.6 ms | -0.0% | [-0.7%, +0.7%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.0 ms | 21.0 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.94 ms | 8.97 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.6 ms | 14.6 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.2 ms | 20.3 ms | +0.4% | [-0.4%, +1.1%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.72 ms | 2.72 ms | -0.1% | [-1.4%, +1.3%] | - | ~ | device 4.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.73 ms | 6.72 ms | -0.1% | [-0.3%, -0.0%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.38 ms | -0.3% | [-0.6%, -0.1%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.73 ms | 2.74 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 7.8x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.53 ms | 6.47 ms | -1.0% | [-1.6%, -0.5%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.7 ms | 33.8 ms | +0.3% | [-0.4%, +1.0%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.6 ms | 68.3 ms | -0.5% | [-1.0%, -0.0%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.4 ms | 18.3 ms | -0.5% | [-0.7%, -0.3%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.7 ms | 20.8 ms | +0.5% | [+0.2%, +0.9%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.2 ms | 38.5 ms | +0.6% | [-0.1%, +1.4%] | - | ~ | device 0.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.8 ms | 12.8 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.65 ms | 9.65 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.8 ms | 12.8 ms | -0.3% | [-0.4%, -0.3%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.30 ms | 3.28 ms | -0.5% | [-1.1%, +0.0%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.58 ms | 2.57 ms | -0.4% | [-1.5%, +0.7%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.56 ms | 4.55 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ | device 2.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.02 ms | 3.02 ms | -0.3% | [-1.0%, +0.4%] | +0.0% | ~ | device 3.3x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 666 µs | 673 µs | +1.0% | [-0.0%, +2.0%] | - | ~ | device 16.7x faster than the host; hand/api 0.95x (was 0.98x) |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.29 ms | 1.28 ms | -0.5% | [-0.9%, -0.0%] | - | ~ | device 14.8x faster than the host; hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 656 µs | 641 µs | -2.3% | [-3.2%, -1.4%] | - | ~ | device 17.4x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.24 ms | +0.8% | [-0.6%, +2.2%] | - | ~ | device 15.0x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 605 µs | 605 µs | -0.0% | [-1.2%, +1.2%] | - | ~ | device 16.1x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.13 ms | +1.0% | [+0.1%, +1.8%] | - | ~ | device 14.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.19 ms | 1.19 ms | -0.6% | [-1.1%, -0.1%] | - | ~ | device 12.0x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.49 ms | 2.49 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 9.8x faster than the host; triangles/polygons 0.51x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.2% | [-0.7%, +1.0%] | - | ~ | device 11.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.19 ms | 2.19 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 9.7x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 628 µs | 596 µs | -5.1% | [-5.4%, -4.9%] | - | faster | device 2.5x faster than the host (was 2.4x); tape/fused 1.15x (was 1.09x) |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 729 µs | 695 µs | -4.7% | [-8.8%, -0.5%] | - | faster | device 2.6x faster than the host (was 2.5x); tape/fused 1.21x (was 1.16x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 685 µs | 685 µs | -0.0% | [-0.7%, +0.7%] | - | ~ | device 3.7x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 845 µs | 842 µs | -0.4% | [-1.0%, +0.2%] | - | ~ | device 3.5x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 16.9 ms | -0.9% | [-1.4%, -0.4%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.4 ms | -0.1% | [-0.6%, +0.5%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.0 ms | 16.9 ms | -0.3% | [-0.9%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.4 ms | 27.5 ms | +0.5% | [-0.2%, +1.3%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.0 ms | 15.0 ms | +0.1% | [-0.7%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.4 ms | -0.5% | [-0.9%, -0.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.5 ms | 16.4 ms | -0.6% | [-1.1%, -0.1%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.4 ms | 24.4 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.1 ms | +1.0% | [-0.2%, +2.2%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 23.0 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.25 ms | 1.25 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 1.3x faster than the host; tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.36 ms | 2.39 ms | +1.1% | [+0.8%, +1.3%] | - | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 1.9x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.49 ms | +0.2% | [-0.4%, +0.8%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.54 ms | 2.54 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 2.6x faster than the host (was 3.0x); marched/traced 3.54x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 2.99 ms | 2.99 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 2.4x faster than the host (was 2.7x); marched/traced 4.63x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.03 ms | 8.99 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 5.1x faster than the host (was 4.4x) |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.8 ms | 13.9 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 3.7x faster than the host (was 3.2x) |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.19 ms | 2.19 ms | -0.4% | [-0.5%, -0.2%] | - | ~ | device 3.9x faster than the host; marched/traced 13.24x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.49 ms | 5.51 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 2.8x faster than the host; marched/traced 10.03x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 29.1 ms | 28.9 ms | -0.5% | [-0.7%, -0.2%] | - | ~ | device 2.3x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 55.2 ms | 55.2 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.22 ms | 5.22 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.77 ms | 8.77 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.17 ms | 6.22 ms | +0.7% | [+0.2%, +1.3%] | - | ~ | device 1.2x faster than the host; marched/traced 25.76x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.5 ms | 10.6 ms | +0.2% | [-0.2%, +0.5%] | - | ~ | device 0.8x faster than the host; marched/traced 24.30x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 163 ms | 160 ms | -1.7% | [-3.3%, -0.1%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 255 ms | 256 ms | +0.4% | [-0.4%, +1.2%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.87 ms | 4.87 ms | +0.0% | [-0.5%, +0.6%] | - | ~ | device 1.8x faster than the host; marched/traced 23.02x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.3 ms | 12.3 ms | -0.2% | [-0.7%, +0.2%] | - | ~ | device 1.3x faster than the host; marched/traced 17.43x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 112 ms | 112 ms | +0.6% | [-0.9%, +2.1%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 214 ms | 214 ms | -0.1% | [-1.6%, +1.4%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.3 ms | 19.3 ms | -0.4% | [-0.7%, -0.2%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.7 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 674 µs | 670 µs | -0.6% | [-1.2%, +0.1%] | - | ~ | device 17.2x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.3% | [-1.1%, +0.5%] | - | ~ | device 15.0x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 674 µs | 675 µs | +0.1% | [-0.5%, +0.8%] | - | ~ | device 17.1x faster than the host; hit/crossing 0.99x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 15.0x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 685 µs | 687 µs | +0.3% | [-0.2%, +0.8%] | - | ~ | device 3.8x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 849 µs | 850 µs | +0.1% | [-0.2%, +0.3%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 608 µs | 609 µs | +0.0% | [-0.1%, +0.2%] | - | ~ | device 3.3x faster than the host; hit/crossing 1.13x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 629 µs | 629 µs | +0.0% | [-0.5%, +0.5%] | - | ~ | device 3.7x faster than the host; hit/crossing 1.35x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.69 ms | 4.68 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 10.1x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 6.9x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.18 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 9.9x faster than the host; hit/crossing 1.12x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.08 ms | 9.08 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 6.7x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 56.6 ms | 56.6 ms | +0.1% | [-1.7%, +1.9%] | - | ~ | device 3.1x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 81.4 ms | 81.4 ms | -0.0% | [-1.0%, +1.0%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 48.2 ms | 48.7 ms | +0.9% | [+0.8%, +1.0%] | - | ~ | device 3.3x faster than the host (was 3.5x); hit/crossing 1.16x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 63.5 ms | 63.6 ms | +0.1% | [-0.6%, +0.9%] | - | ~ | device 2.4x faster than the host; hit/crossing 1.28x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.0 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.5 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.1 ms | 17.1 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.3% | [-0.6%, +0.0%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.6% | [+0.3%, +1.0%] | - | ~ | device 1.9x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.49 ms | -0.2% | [-0.4%, -0.1%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.11 ms | 1.11 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 1.9x faster than the host; hit/crossing 1.26x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.16 ms | 1.16 ms | +0.3% | [-0.0%, +0.6%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.14x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.4 ms | 64.5 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 123 ms | 122 ms | -0.9% | [-3.5%, +1.7%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.6 ms | 58.5 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 111 ms | 113 ms | +1.4% | [-0.5%, +3.4%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.08x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 493 ms | 483 ms | -2.1% | [-3.7%, -0.4%] | - | ~ | device 0.4x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 712 ms | 718 ms | +0.9% | [-2.2%, +4.0%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 395 ms | 387 ms | -1.9% | [-6.0%, +5.9%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.25x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 527 ms | 518 ms | -1.7% | [-3.3%, -0.1%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.39x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.59 ms | 3.58 ms | -0.3% | [-3.6%, +3.1%] | - | ~ | device 1.2x faster than the host; marched/traced 3.36x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.77 ms | 4.75 ms | -0.3% | [-1.3%, +0.7%] | - | ~ | device 0.9x faster than the host; marched/traced 5.59x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.0 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 3.9x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.6 ms | 26.6 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 2.0x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.24 ms | 1.23 ms | -0.5% | [-2.1%, +1.1%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.84 ms | 1.85 ms | +0.1% | [-0.9%, +1.1%] | - | ~ | device 1.7x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 94.7 µs | 95.9 µs | +1.3% | [+0.9%, +1.8%] | - | ~ | device 15.1x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 152 µs | 153 µs | +1.1% | [+0.6%, +1.6%] | - | ~ | device 10.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 792 µs | 794 µs | +0.3% | [-0.3%, +0.9%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 929 µs | 930 µs | +0.0% | [-0.3%, +0.4%] | - | ~ | device 2.8x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 34.3 ms | 35.3 ms | +2.9% | [-3.8%, +4.3%] | - | ~ | device 0.1x faster than the host; marched/traced 3.39x (was 3.55x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.5 ms | 36.6 ms | +0.4% | [-0.4%, +1.2%] | - | ~ | device 0.1x faster than the host; marched/traced 6.48x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 122 ms | 120 ms | -1.7% | [-3.1%, -0.4%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 233 ms | 237 ms | +1.7% | [-1.6%, +5.1%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 17.1 ms | 17.1 ms | -0.1% | [-2.2%, +2.2%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.8 ms | 21.9 ms | +0.4% | [-0.4%, +1.2%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.06 ms | 1.02 ms | -3.6% | [-7.3%, +0.2%] | - | ~ | device 1.5x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.36 ms | 2.33 ms | -1.2% | [-2.6%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.96 ms | 5.97 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.72 ms | 6.72 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.5 ms | 23.0 ms | -2.3% | [-4.1%, -0.5%] | - | ~ | device 2.2x faster than the host; marched/traced 2.71x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.6 ms | 28.3 ms | -0.9% | [-1.8%, -0.1%] | - | ~ | device 1.9x faster than the host; marched/traced 3.71x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 62.5 ms | 62.3 ms | -0.3% | [-0.7%, +0.0%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 105 ms | 105 ms | -0.2% | [-1.2%, +0.9%] | - | ~ | device 2.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.7 ms | 13.6 ms | -0.7% | [-1.5%, +0.1%] | - | ~ | device 3.7x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.4 ms | 17.3 ms | -0.5% | [-1.2%, +0.2%] | - | ~ | device 3.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 306 ms | 301 ms | -1.7% | [-7.4%, +4.2%] | - | ~ | device 0.2x faster than the host; marched/traced 1.86x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 349 ms | 353 ms | +1.2% | [-0.3%, +2.7%] | - | ~ | device 0.2x faster than the host; marched/traced 2.82x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 554 ms | 559 ms | +1.0% | [+0.1%, +1.9%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 991 ms | 997 ms | +0.6% | [-0.3%, +1.6%] | - | ~ | device 0.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 191 ms | 188 ms | -1.5% | [-5.1%, +2.2%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 238 ms | 237 ms | -0.6% | [-2.4%, +2.4%] | - | ~ | device 0.2x faster than the host |
| `Regions/particles/region/float/hostdevice` | - | - | - | - | - | new |  |
| `Regions/particles/regionDistances/float/hostdevice` | - | - | - | - | - | new |  |
| `Regions/particles/regionByRegion/float/hostdevice` | - | - | - | - | - | new | each/one-pass 0.04x |
| `Regions/particles/domainLabels/float/hostdevice` | - | - | - | - | - | new |  |
| `Regions/assembly/firstRegionChange/float/hostdevice` | - | - | - | - | - | new |  |
| `Regions/assembly/firstHit/float/hostdevice` | - | - | - | - | - | new | regions/hit 1.20x |
| `Regions/particles/region/double/hostdevice` | - | - | - | - | - | new |  |
| `Regions/particles/regionDistances/double/hostdevice` | - | - | - | - | - | new |  |
| `Regions/particles/regionByRegion/double/hostdevice` | - | - | - | - | - | new | each/one-pass 0.04x |
| `Regions/particles/domainLabels/double/hostdevice` | - | - | - | - | - | new |  |
| `Regions/assembly/firstRegionChange/double/hostdevice` | - | - | - | - | - | new |  |
| `Regions/assembly/firstHit/double/hostdevice` | - | - | - | - | - | new | regions/hit 1.22x |
| `Render/csg/frame/float/hostdevice` | 4.29 ms | 3.99 ms | -7.0% | [-7.1%, -7.0%] | - | faster | device 2.8x faster than the host (was 2.6x); checksum differs (196590.4848 -> 197096.378) |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.29 ms | 3.99 ms | -7.0% | [-7.0%, -6.9%] | - | faster | device 2.9x faster than the host (was 2.6x); checksum differs (194522.8835 -> 194925.7287) |
| `Render/mesh/frame/float/hostdevice` | 1.89 ms | 1.97 ms | +4.5% | [+3.7%, +5.6%] | - | minor | device 8.5x faster than the host (was 8.9x); checksum differs (214206.9999 -> 214324.6688) |
| `Render/csg/frame/double/hostdevice` | 7.66 ms | 7.66 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 1.7x faster than the host; checksum differs (196590.4821 -> 197096.379) |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.66 ms | 7.66 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 1.7x faster than the host; checksum differs (194522.8728 -> 194925.718) |
| `Render/mesh/frame/double/hostdevice` | 23.2 ms | 23.3 ms | +0.5% | [-0.2%, +1.3%] | - | ~ | device 0.7x faster than the host; checksum differs (214207.0052 -> 214324.674) |
| `Shapes/ellipsoid/signedDistance/float/hostdevice` | 156 µs | 156 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 33.0x faster than the host |
| `Shapes/capped-cone/signedDistance/float/hostdevice` | 47.7 µs | 48.2 µs | +1.0% | [-0.6%, +2.7%] | - | ~ | device 18.3x faster than the host (was 19.3x) |
| `Shapes/round-cone/signedDistance/float/hostdevice` | 47.9 µs | 47.8 µs | -0.2% | [-0.3%, -0.0%] | - | ~ | device 15.7x faster than the host |
| `Shapes/polygon-prism/signedDistance/float/hostdevice` | 48.6 µs | 48.1 µs | -0.9% | [-2.0%, +0.1%] | - | ~ | device 65.7x faster than the host |
| `Shapes/tube/signedDistance/float/hostdevice` | 48.1 µs | 47.9 µs | -0.5% | [-1.3%, +0.2%] | - | ~ | device 15.6x faster than the host |
| `Shapes/capped-torus/signedDistance/float/hostdevice` | 48.1 µs | 48.0 µs | -0.2% | [-1.1%, +0.8%] | - | ~ | device 17.6x faster than the host |
| `Shapes/cut-sphere/signedDistance/float/hostdevice` | 48.4 µs | 48.2 µs | -0.4% | [-1.6%, +0.8%] | - | ~ | device 9.6x faster than the host |
| `Shapes/cut-hollow-sphere/signedDistance/float/hostdevice` | 47.8 µs | 48.0 µs | +0.5% | [+0.2%, +0.8%] | - | ~ | device 8.1x faster than the host |
| `Shapes/wedge/signedDistance/float/hostdevice` | 48.5 µs | 48.7 µs | +0.4% | [-0.3%, +1.2%] | - | ~ | device 8.2x faster than the host (was 8.6x) |
| `Shapes/ray-forms/firstHit/float/hostdevice` | 553 µs | 555 µs | +0.3% | [-0.1%, +0.7%] | - | ~ | device 8.3x faster than the host |
| `Shapes/ellipsoid/signedDistance/double/hostdevice` | 7.23 ms | 7.23 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.0x faster than the host |
| `Shapes/capped-cone/signedDistance/double/hostdevice` | 493 µs | 493 µs | -0.1% | [-0.4%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `Shapes/round-cone/signedDistance/double/hostdevice` | 421 µs | 421 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.8x faster than the host (was 1.9x) |
| `Shapes/polygon-prism/signedDistance/double/hostdevice` | 1.69 ms | 1.69 ms | -0.0% | [-0.1%, -0.0%] | - | ~ | device 2.4x faster than the host |
| `Shapes/tube/signedDistance/double/hostdevice` | 358 µs | 358 µs | -0.1% | [-0.2%, -0.0%] | - | ~ | device 2.3x faster than the host |
| `Shapes/capped-torus/signedDistance/double/hostdevice` | 669 µs | 669 µs | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.7x faster than the host |
| `Shapes/cut-sphere/signedDistance/double/hostdevice` | 414 µs | 414 µs | -0.0% | [-0.1%, +0.1%] | - | ~ | device 1.4x faster than the host |
| `Shapes/cut-hollow-sphere/signedDistance/double/hostdevice` | 339 µs | 339 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.3x faster than the host (was 1.4x) |
| `Shapes/wedge/signedDistance/double/hostdevice` | 155 µs | 154 µs | -0.1% | [-0.3%, +0.2%] | - | ~ | device 3.0x faster than the host (was 2.8x) |
| `Shapes/ray-forms/firstHit/double/hostdevice` | 8.04 ms | 8.03 ms | -0.2% | [-0.3%, +0.0%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 890 µs | 888 µs | -0.2% | [-0.9%, +0.4%] | +0.0% | ~ | device 10.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.64 ms | 2.64 ms | +0.2% | [-0.0%, +0.5%] | +0.0% | ~ | device 4.8x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.5 ms | 13.5 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | device 3.6x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.1 ms | 10.2 ms | +0.6% | [+0.5%, +0.6%] | +0.0% | ~ | device 3.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.1 ms | 13.1 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.4 ms | 14.4 ms | +0.3% | [-0.3%, +0.9%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 13.9 ms | 14.0 ms | +0.3% | [+0.0%, +0.6%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.0 ms | 11.1 ms | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 2.99 ms | 3.01 ms | +0.7% | [-0.1%, +1.5%] | +0.0% | ~ | device 7.2x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.72 ms | 4.72 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | device 5.1x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | device 5.2x faster than the host; tape/fused 1.37x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.0 ms | 24.0 ms | +0.1% | [+0.0%, +0.3%] | +0.0% | ~ | device 4.2x faster than the host (was 3.8x); tape/fused 1.26x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 22.9 ms | 22.9 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.3 ms | 30.3 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ | device 3.0x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.3 ms | 44.1 ms | -0.3% | [-0.8%, +0.1%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.7 ms | 45.7 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 126 ms | 122 ms | -3.7% | [-4.4%, -2.9%] | +0.0% | faster | device 0.7x faster than the host (was 0.7x); tape/fused 1.04x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 127 ms | 127 ms | -0.6% | [-3.3%, +2.2%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 129 ms | 127 ms | -1.9% | [-2.9%, -0.8%] | +0.0% | ~ | device 0.7x faster than the host (was 0.7x) |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 136 ms | 131 ms | -3.2% | [-5.1%, -1.3%] | +0.0% | faster | device 0.8x faster than the host (was 0.7x) |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.59 ms | 1.59 ms | +0.2% | [+0.0%, +0.3%] | +0.0% | ~ | device 10.6x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 31.1 ms | 31.2 ms | +0.1% | [-0.0%, +0.3%] | +0.0% | ~ | device 0.6x faster than the host |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | 9.31 ms | 9.33 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 11.6x faster than the host |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | 5.93 ms | 5.92 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 10.9x faster than the host; surface/leaf 1.57x |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | 94.6 ms | 96.0 ms | +1.5% | [-0.0%, +3.0%] | - | ~ | device 3.5x faster than the host (was 3.7x) |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | 63.3 ms | 63.2 ms | -0.2% | [-0.2%, -0.1%] | - | ~ | device 3.2x faster than the host; surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | 29.2 ms | 29.1 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 5.0x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | 9.89 ms | 9.89 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 6.9x faster than the host; surface/hit 2.95x |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | 59.0 ms | 59.1 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 1.8x faster than the host |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | 38.9 ms | 38.9 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.5x faster than the host; surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | 782 ms | 782 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | 528 ms | 528 ms | -0.0% | [-0.5%, +0.5%] | - | ~ | device 0.4x faster than the host; surface/leaf 1.48x |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | 307 ms | 307 ms | +0.2% | [-0.8%, +1.2%] | - | ~ | device 0.5x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | 119 ms | 121 ms | +2.0% | [+1.0%, +3.1%] | - | ~ | device 0.6x faster than the host; surface/hit 2.53x |
| `Tape/csg/signedDistance/float/hostdevice` | 218 µs | 218 µs | -0.0% | [-0.3%, +0.2%] | - | ~ | device 11.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.8 µs | -0.2% | [-0.3%, -0.0%] | - | ~ | device 23.6x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 359 µs | 359 µs | -0.1% | [-0.2%, +0.1%] | - | ~ | device 12.8x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.56 ms | 1.56 ms | +0.1% | [+0.0%, +0.3%] | - | ~ | device 9.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 71.9 µs | 72.4 µs | +0.6% | [+0.4%, +0.9%] | - | ~ | device 7.3x faster than the host (was 7.7x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 48.2 µs | 48.3 µs | +0.2% | [-1.2%, +1.5%] | - | ~ | device 5.0x faster than the host (was 4.4x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 653 µs | +0.1% | [-0.1%, +0.4%] | - | ~ | device 10.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | -0.0% | [-0.3%, +0.3%] | - | ~ | device 11.6x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 477 µs | 477 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 7.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.5 µs | 48.4 µs | -0.3% | [-0.9%, +0.3%] | - | ~ | device 27.5x faster than the host (was 26.7x) |
| `Tape/csg/signedDistance/double/hostdevice` | 859 µs | 860 µs | +0.1% | [-0.4%, +0.5%] | - | ~ | device 3.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 811 µs | +0.1% | [-0.0%, +0.3%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.65 ms | 6.64 ms | -0.2% | [-0.2%, -0.1%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 191 µs | 192 µs | +0.1% | [+0.0%, +0.2%] | - | ~ | device 3.0x faster than the host (was 2.9x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | -0.0% | [-0.2%, +0.1%] | - | ~ | device 1.9x faster than the host |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.12 ms | 2.12 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.9x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.61 ms | 1.61 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 8.1x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.71 ms | 3.71 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 10.9x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.50 ms | 6.51 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 17.8 ms | 17.8 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 4 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4227.3 s → 3693.0 s (-12.6%), the largest peak 5633 MB → 5631 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 12.5 s | 11.9 s | -4.5% | 547 MB | 591 MB | +7.9% | ~ |
| `BVHRefit` | 6.1 s | 6.1 s | -1.4% | 474 MB | 501 MB | +5.8% | ~ |
| `BVHUnion` | 24.0 s | 22.2 s | -7.7% | 972 MB | 1017 MB | +4.6% | ~ |
| `BVHUnionMixed` | 10.3 s | 9.9 s | -3.8% | 571 MB | 616 MB | +7.8% | ~ |
| `ClosestPrimitive` | 128.6 s | 111.0 s | -13.7% | 1242 MB | 1339 MB | +7.8% | ~ |
| `ClosestSurface` | 196.1 s | 158.9 s | -18.9% | 1861 MB | 1843 MB | -1.0% | ~ |
| `Fused` | 49.8 s | 47.1 s | -5.3% | 1052 MB | 1149 MB | +9.2% | ~ |
| `LaunchTuning` | 521.0 s | 450.5 s | -13.5% | 3806 MB | 3806 MB | -0.0% | ~ |
| `Mesh` | 19.6 s | 19.0 s | -3.0% | 750 MB | 792 MB | +5.7% | ~ |
| `MeshCSG` | 354.1 s | 313.1 s | -11.6% | 5103 MB | 5162 MB | +1.1% | ~ |
| `MeshReconcile` | 9.5 s | 9.4 s | -0.9% | 474 MB | 503 MB | +6.1% | ~ |
| `PointCloud` | 11.7 s | 11.7 s | +0.6% | 516 MB | 560 MB | +8.7% | ~ |
| `PointCloudBVHBuild` | 6.2 s | 6.2 s | -1.1% | 474 MB | 501 MB | +5.8% | ~ |
| `PointCloudHashGridBuild` | 5.5 s | 5.6 s | +1.9% | 474 MB | 502 MB | +5.8% | ~ |
| `Ray` | 187.1 s | 162.5 s | -13.1% | 3703 MB | 3703 MB | -0.0% | ~ |
| `RayCSG` | 220.3 s | 188.1 s | -14.7% | 4086 MB | 4085 MB | -0.0% | ~ |
| `RayCrossing` | 429.4 s | 358.7 s | -16.5% | 4019 MB | 4016 MB | -0.1% | ~ |
| `RayParticles` | 658.6 s | 604.5 s | -8.2% | 5633 MB | 5631 MB | -0.0% | ~ |
| `RayShell` | 393.8 s | 334.6 s | -15.0% | 5346 MB | 5345 MB | -0.0% | ~ |
| `Regions` | - | 297.9 s | | - | 3878 MB | | new |
| `Render` | 213.7 s | 190.9 s | -10.7% | 4937 MB | 5217 MB | +5.7% | ~ |
| `Shapes` | 27.0 s | 22.9 s | -15.3% | 536 MB | 581 MB | +8.4% | ~ |
| `SphereCloudBVH` | 7.9 s | 7.2 s | -8.6% | 474 MB | 502 MB | +5.8% | ~ |
| `SphereCloudRays` | 184.2 s | 160.7 s | -12.7% | 4095 MB | 4094 MB | -0.0% | ~ |
| `SphereContacts` | 7.2 s | 6.9 s | -5.3% | 474 MB | 501 MB | +5.8% | ~ |
| `SurfaceParts` | 437.6 s | 380.9 s | -13.0% | 4336 MB | 4242 MB | -2.2% | ~ |
| `Tape` | 65.7 s | 57.2 s | -13.0% | 876 MB | 972 MB | +11.0% | ~ |
| `TapeBVHUnion` | 39.8 s | 35.3 s | -11.3% | 819 MB | 913 MB | +11.5% | ~ |
| **all, in both** | 4227.3 s | 3693.0 s | -12.6% | 5633 MB | 5631 MB | -0.0% | ~ |

Device kernels: 178 in both, 16 changed (8 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 12 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Fused::Node<Fused::SmoothCombineOp<…` | 168 → 168 | 4720 → 4976 | 860/732 → 1104/920 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double, TriMeshSDF<double, 4ul…` | 168 → 168 | 19120 → 19376 | 1048/1072 → 1388/1260 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Fused::Node<Fused::SmoothCombineOp<f…` | 168 → 168 | 2776 → 2992 | 84/104 → 368/488 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float, TriMeshSDF<float, 4ul, 4…` | 168 → 168 | 13104 → 13360 | 720/736 → 1280/948 | **spills** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double> >, unsigned int const,…` | 168 → 168 | 18736 → 18992 | 1052/1076 → 1388/1260 | **spills** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, TriMeshSDF<double, 4ul, 4ul> >, uns…` | 168 → 168 | 1560 → 1840 | 660/572 → 764/724 | **spills stack** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float> >, unsigned int const, f…` | 168 → 168 | 13312 → 13552 | 728/740 → 1284/952 | **spills** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, TriMeshSDF<float, 4ul, 4ul> >, unsig…` | 160 → 168 | 984 → 1136 | 0/0 → 12/12 | **registers spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<double, Tape<double, BVHUnion<double, 4ul, …` | 168 → 168 | 7936 → 7744 | 80/260 → 48/152 | ~ |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<double, Tape<double, SphereCloudBVH<double,…` | 168 → 168 | 8032 → 7728 | 80/260 → 48/152 | ~ |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<float, Tape<float, BVHUnion<float, 4ul, Sph…` | 168 → 168 | 5008 → 4848 | 96/256 → 44/164 | ~ |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<float, Tape<float, SphereCloudBVH<float, 4u…` | 168 → 168 | 5088 → 4816 | 96/256 → 44/164 | ~ |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<double, Tape<double, BVHUnion<double, …` | 168 → 168 | 7872 → 7680 | 64/192 → 28/128 | ~ |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<double, Tape<double, TriMeshSDF<double…` | 168 → 168 | 7744 → 7712 | 80/204 → 28/128 | ~ |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<float, Tape<float, BVHUnion<float, 4ul…` | 168 → 168 | 4976 → 4768 | 96/268 → 28/140 | ~ |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<float, Tape<float, TriMeshSDF<float, 4…` | 168 → 168 | 4784 → 4768 | 104/244 → 28/140 | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (5 cases): MeshCSG/smooth-fused/firstHit-morton/double/hostdevice, RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice, RayParticles/armadillo-shell-tape/step-morton/double/hostdevice, RayShell/armadillo-fused/firstHit-random/double/hostdevice, Render/mesh/frame/float/hostdevice

The probes differ between the sides (each side ran its own): Performance/Regions/main.cpp, Performance/SurfaceParts/main.cpp, Performance/Common/Inputs.hpp

<!-- END receipt cuda -->
