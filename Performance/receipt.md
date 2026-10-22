<!--
SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>

SPDX-License-Identifier: GPL-3.0-or-later
-->

<!-- Written by Scripts/run-performance.sh compare --record and checked by CI (Performance/CheckReceipt.cmake).
     Commit it with the pull request. Do not edit it, except to accept a yellow or red verdict: write the
     reason after "accepted:" in that section. See Docs/Sphinx/source/PerformanceTesting.rst. -->

# Performance receipt

<!-- BEGIN receipt host -->
<!--receipt
version: 2
side: host
verdict: yellow
accepted: Algorithm changed
base: d3d6d00
base-fingerprint: 5d486a59bfb4cdfdfbfe20bbd6c0c73f42a9e748
head: 8cf9be2
head-fingerprint: 8881f2387dcb870b05bfe4fdbc764c733fcd5048
cases: 106
rounds: 3
reps: 5
date: 2026-10-22
-->

## Host comparison: yellow

`8cf9be2` against `d3d6d00` (origin/claude/renderer-examples), 2026-10-22.

**VERDICT: no regression, but 4 checksums changed: check those answers (see Flagged above)**

Performance comparison: 0 major, 0 minor, 4 faster, 0 differ, 0 lost, 106 cases

- base: d3d6d00 (gcc 13.3.0, simd avx2, gpu host)
- head: 8cf9be2 (gcc 13.3.0, simd avx2, gpu host)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/build/float/host` | 53.1 ms | 53.7 ms | +1.1% | [-0.3%, +2.6%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 49.0 ms | 49.0 ms | -0.1% | [-2.7%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 61.9 ms | 61.7 ms | -0.3% | [-0.6%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 53.3 ms | 53.4 ms | +0.3% | [-0.7%, +1.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 57.9 ms | 57.8 ms | -0.1% | [-0.7%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 70.4 ms | 70.6 ms | +0.3% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 62.4 ms | 62.2 ms | -0.4% | [-0.8%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 79.1 ms | 79.1 ms | +0.0% | [-21.6%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 63.7 ms | 64.1 ms | +0.7% | [-0.5%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 98.2 ms | 98.2 ms | -0.1% | [-2.0%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 74.8 ms | 74.8 ms | -0.1% | [-1.6%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 107 ms | 107 ms | -0.0% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 70.3 ms | 70.4 ms | +0.1% | [-0.1%, +1.8%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 241 ms | 241 ms | +0.0% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 84.3 ms | 84.4 ms | +0.1% | [-0.7%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 279 ms | 281 ms | +0.9% | [-2.3%, +2.8%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 59.6 ms | 59.7 ms | +0.2% | [-0.5%, +0.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 213 ms | 212 ms | -0.2% | [-0.5%, +5.0%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 82.0 ms | 81.6 ms | -0.6% | [-1.2%, +0.4%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.7 ms | 29.9 ms | +0.8% | [-2.6%, +2.7%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 753 ms | 754 ms | +0.1% | [-1.9%, +2.3%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 98.2 ms | 97.4 ms | -0.8% | [-1.0%, -0.1%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 42.7 ms | 42.3 ms | -1.0% | [-5.9%, +1.0%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 840 ms | 840 ms | -0.1% | [-0.6%, +0.3%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/build/float/host` | 65.5 ms | 65.4 ms | -0.2% | [-0.7%, +0.0%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 147 ms | 148 ms | +0.4% | [-0.5%, +0.9%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 364 ms | 366 ms | +0.4% | [-0.6%, +0.9%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 77.6 ms | 77.9 ms | +0.4% | [-0.1%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.61 ms | 6.59 ms | -0.2% | [-0.6%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/float/host` | 144 ms | 144 ms | +0.3% | [-0.2%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/float/host` | 334 ms | 333 ms | -0.2% | [-0.2%, +1.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/float/host` | 182 ms | 182 ms | -0.0% | [-0.8%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 82.6 ms | 83.5 ms | +1.1% | [+0.5%, +2.5%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 165 ms | 164 ms | -0.6% | [-1.0%, +1.9%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 411 ms | 410 ms | -0.1% | [-0.7%, +1.0%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 86.9 ms | 86.7 ms | -0.3% | [-1.3%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.97 ms | 6.94 ms | -0.5% | [-0.7%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/double/host` | 156 ms | 156 ms | -0.2% | [-3.1%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/double/host` | 356 ms | 356 ms | +0.1% | [-1.3%, +1.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/double/host` | 191 ms | 190 ms | -0.4% | [-0.5%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 11.4 ms | 11.5 ms | +0.3% | [-2.0%, +2.4%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 123 ms | 123 ms | +0.4% | [-0.2%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 535 ms | 530 ms | -0.9% | [-2.3%, -0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 152 ms | 153 ms | +0.2% | [-0.3%, +1.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 856 ms | 857 ms | +0.0% | [-2.9%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/float/host` | 120 ms | 120 ms | +0.2% | [-0.0%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/host` | 510 ms | 510 ms | -0.0% | [-0.1%, +0.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/float/host` | 359 ms | 359 ms | +0.0% | [-0.2%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 9.06 ms | 9.06 ms | +0.0% | [-1.9%, +1.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 126 ms | 126 ms | -0.2% | [-1.2%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 558 ms | 558 ms | +0.1% | [-0.3%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 156 ms | 156 ms | -0.2% | [-9.5%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 941 ms | 942 ms | +0.2% | [-0.2%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/double/host` | 120 ms | 121 ms | +0.5% | [+0.3%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/host` | 511 ms | 512 ms | +0.1% | [-0.4%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/double/host` | 361 ms | 362 ms | +0.2% | [-0.2%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 64.9 ms | 65.3 ms | +0.5% | [-0.1%, +3.5%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 109 ms | 109 ms | +0.2% | [-1.3%, +0.9%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 266 ms | 266 ms | +0.1% | [-0.4%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 112 ms | 112 ms | -0.0% | [-2.0%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 105 ms | 105 ms | +0.0% | [-0.4%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/float/host` | 107 ms | 107 ms | +0.1% | [-1.1%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/float/host` | 235 ms | 236 ms | +0.3% | [-0.1%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/float/host` | 191 ms | 192 ms | +0.1% | [-0.5%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 81.7 ms | 80.9 ms | -0.9% | [-1.2%, -0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 123 ms | 125 ms | +1.1% | [-2.3%, +2.1%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 305 ms | 305 ms | -0.2% | [-0.7%, +0.9%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 127 ms | 127 ms | +0.5% | [-2.6%, +1.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 121 ms | 121 ms | -0.1% | [-0.7%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/double/host` | 117 ms | 117 ms | +0.0% | [-2.2%, +1.3%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/double/host` | 254 ms | 252 ms | -0.9% | [-1.4%, +2.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/double/host` | 200 ms | 199 ms | -0.6% | [-0.7%, -0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 8.73 ms | 8.68 ms | -0.6% | [-2.2%, +3.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 142 ms | 142 ms | +0.2% | [-0.2%, +5.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 406 ms | 407 ms | +0.4% | [-3.2%, +3.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 253 ms | 252 ms | -0.3% | [-1.2%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 333 ms | 334 ms | +0.2% | [-0.3%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/float/host` | 146 ms | 147 ms | +0.4% | [-4.5%, +6.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/host` | 385 ms | 386 ms | +0.2% | [-0.7%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/float/host` | 354 ms | 354 ms | +0.0% | [-1.9%, +2.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 6.34 ms | 6.32 ms | -0.4% | [-3.7%, +1.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 147 ms | 145 ms | -1.4% | [-3.9%, +4.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 422 ms | 422 ms | +0.2% | [-0.2%, +3.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 287 ms | 287 ms | +0.3% | [-1.4%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 371 ms | 371 ms | -0.0% | [-1.0%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/double/host` | 142 ms | 142 ms | -0.0% | [-4.1%, +4.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/host` | 374 ms | 373 ms | -0.2% | [-5.3%, +3.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/double/host` | 346 ms | 352 ms | +2.0% | [-2.5%, +2.3%] | - | ~ |  |
| `Render/csg/firstHit/float/host` | 19.0 ms | 12.7 ms | -33.1% | [-36.3%, -29.3%] | +0.0% | faster | checksum differs (303538.3704 -> 303212.6064) |
| `Render/union64/firstHit/float/host` | - | - | - | - | - | new |  |
| `Render/mixed/firstHit/float/host` | - | - | - | - | - | new |  |
| `Render/mesh/firstHit/float/host` | 1.69 ms | 1.68 ms | -0.3% | [-0.7%, +0.5%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 26.4 ms | 15.3 ms | -41.9% | [-42.0%, -41.4%] | +0.0% | faster | checksum differs (1.288592882e+14 -> 1.293817267e+14) |
| `Render/csg/firstHit/double/host` | 19.1 ms | 13.8 ms | -27.6% | [-28.0%, -21.2%] | +0.0% | faster | checksum differs (303527.4187 -> 303206.4785) |
| `Render/union64/firstHit/double/host` | - | - | - | - | - | new |  |
| `Render/mixed/firstHit/double/host` | - | - | - | - | - | new |  |
| `Render/mesh/firstHit/double/host` | 1.77 ms | 1.72 ms | -2.7% | [-6.5%, +3.1%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 26.6 ms | 16.5 ms | -38.0% | [-39.6%, -37.8%] | +0.0% | faster | checksum differs (1.288588646e+14 -> 1.293817097e+14) |
| `Tape/csg/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Tape/csg-handwritten/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Tape/smooth/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Tape/union64/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Tape/csg/signedDistance/double/host` | - | - | - | - | - | new |  |
| `Tape/csg-handwritten/signedDistance/double/host` | - | - | - | - | - | new |  |
| `Tape/smooth/signedDistance/double/host` | - | - | - | - | - | new |  |
| `Tape/union64/signedDistance/double/host` | - | - | - | - | - | new |  |


The probes differ between the sides (each side ran its own): Performance/Render/main.cpp, Performance/Tape/main.cpp, Performance/Common/Inputs.hpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 2
side: cuda
verdict: yellow
accepted: Algorithm changed
base: d3d6d00
base-fingerprint: 5d486a59bfb4cdfdfbfe20bbd6c0c73f42a9e748
head: 8cf9be2
head-fingerprint: 8881f2387dcb870b05bfe4fdbc764c733fcd5048
cases: 76
rounds: 3
reps: 5
date: 2026-10-22
-->

## Device comparison (CUDA): yellow

`8cf9be2` against `d3d6d00` (origin/claude/renderer-examples), 2026-10-22.

**VERDICT: no regression, but 2 checksums changed: check those answers (see Flagged above)**

Performance comparison: 0 major, 0 minor, 2 faster, 0 differ, 0 lost, 76 cases

- base: d3d6d00 (gcc 13.3.0, simd none, gpu CUDA)
- head: 8cf9be2 (gcc 13.3.0, simd none, gpu CUDA)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/query-morton/float/hostdevice` | 417 µs | 416 µs | -0.3% | [-6.5%, +0.5%] | - | ~ | device 13.1x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 941 µs | 934 µs | -0.8% | [-2.7%, +0.7%] | - | ~ | device 8.6x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.41 ms | 4.42 ms | +0.1% | [-0.9%, +0.9%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.55 ms | 4.56 ms | +0.3% | [-1.3%, +0.7%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 698 µs | 693 µs | -0.6% | [-0.7%, +1.1%] | - | ~ | device 10.7x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.59 ms | 1.59 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 7.0x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.63 ms | 6.61 ms | -0.2% | [-0.6%, +0.6%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 6.86 ms | 6.87 ms | +0.2% | [-0.2%, +0.5%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 1.12 ms | 1.12 ms | -0.2% | [-0.4%, +0.5%] | - | ~ | device 11.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.70 ms | 2.66 ms | -1.2% | [-1.6%, +0.3%] | - | ~ | device 6.6x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.29 ms | 9.31 ms | +0.2% | [-0.4%, +0.7%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 9.87 ms | 9.89 ms | +0.2% | [-0.4%, +0.5%] | - | ~ | device 1.8x faster than the host |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 9.26 ms | 9.11 ms | -1.6% | [-5.6%, +5.1%] | - | ~ | device 10.3x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 22.1 ms | 22.2 ms | +0.3% | [-0.7%, +0.7%] | - | ~ | device 5.4x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 141 ms | 141 ms | +0.5% | [-1.2%, +0.9%] | - | ~ | device 0.7x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 264 ms | 263 ms | -0.5% | [-2.5%, +2.9%] | - | ~ | device 0.5x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 28.8 ms | 28.6 ms | -0.7% | [-1.1%, -0.5%] | - | ~ | device 3.4x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 68.0 ms | 68.0 ms | -0.1% | [-0.3%, +0.3%] | - | ~ | device 2.1x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 138 ms | 136 ms | -1.3% | [-2.4%, +3.3%] | - | ~ | device 0.7x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 228 ms | 228 ms | -0.3% | [-3.4%, +4.3%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 781 µs | 781 µs | +0.0% | [-0.5%, +0.7%] | - | ~ | device 10.3x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 3.09 ms | 3.09 ms | +0.0% | [-1.7%, +0.7%] | - | ~ | device 4.0x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.16 ms | 1.17 ms | +0.3% | [-0.6%, +0.3%] | - | ~ | device 8.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 11.2 ms | 11.2 ms | -0.1% | [-0.5%, +0.1%] | - | ~ | device 2.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 6.60 ms | 6.63 ms | +0.6% | [-0.4%, +1.1%] | - | ~ | device 2.6x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 6.97 ms | 6.93 ms | -0.6% | [-1.3%, +0.0%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.74 ms | 7.73 ms | -0.1% | [-0.9%, +0.9%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.2 ms | 13.2 ms | +0.5% | [-0.1%, +16.5%] | - | ~ | device 0.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 25.5 ms | 25.4 ms | -0.5% | [-0.8%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 14.8 ms | 14.7 ms | -0.6% | [-1.6%, +0.5%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 2.03 ms | 2.03 ms | -0.2% | [-0.9%, +1.7%] | - | ~ | device 5.1x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 5.16 ms | 5.17 ms | +0.1% | [-1.2%, +0.9%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 859 µs | 860 µs | +0.2% | [-8.2%, +8.7%] | - | ~ | device 10.0x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 11.6 ms | 11.7 ms | +0.8% | [-0.6%, +2.2%] | - | ~ | device 2.8x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 7.33 ms | 7.35 ms | +0.4% | [-2.5%, +0.6%] | - | ~ | device 3.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.25 ms | 8.30 ms | +0.6% | [-1.5%, +2.2%] | - | ~ | device 1.3x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 16.1 ms | 13.8 ms | -14.5% | [-16.7%, +7.3%] | - | ~ | device 1.2x faster than the host (was 1.0x) |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 3.37 ms | 3.37 ms | -0.1% | [-1.4%, +1.0%] | - | ~ | device 2.6x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 24.9 ms | 24.8 ms | -0.1% | [-0.8%, +0.1%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 16.7 ms | 16.5 ms | -1.2% | [-2.0%, +0.7%] | - | ~ | device 1.6x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.48 ms | 1.48 ms | -0.0% | [-0.9%, +1.7%] | - | ~ | device 8.6x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 6.46 ms | 6.47 ms | +0.1% | [-0.8%, +1.0%] | - | ~ | device 2.9x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 1.37 ms | 1.36 ms | -1.2% | [-10.9%, +11.7%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 5.66 ms | 5.67 ms | +0.0% | [-1.3%, +2.3%] | - | ~ | device 3.3x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 7.55 ms | 7.55 ms | -0.0% | [-0.2%, +0.6%] | - | ~ | device 2.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 13.2 ms | 12.0 ms | -9.7% | [-11.0%, +0.4%] | - | ~ | device 1.1x faster than the host (was 1.0x) |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 15.6 ms | 15.6 ms | -0.0% | [-3.0%, +1.6%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 6.47 ms | 6.45 ms | -0.5% | [-2.3%, +1.6%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 15.4 ms | 15.4 ms | -0.5% | [-1.0%, +0.4%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 18.5 ms | 18.4 ms | -0.8% | [-1.2%, +1.1%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 6.38 ms | 6.38 ms | -0.0% | [-2.2%, +3.1%] | - | ~ | device 2.7x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 18.1 ms | 18.2 ms | +0.2% | [-0.9%, +0.5%] | - | ~ | device 1.9x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.91 ms | 1.91 ms | -0.0% | [-9.1%, +10.6%] | - | ~ | device 9.3x faster than the host (was 6.1x) |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 5.80 ms | 5.80 ms | -0.0% | [-3.8%, +4.6%] | - | ~ | device 4.7x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 14.8 ms | 14.9 ms | +0.6% | [-1.7%, +1.3%] | - | ~ | device 1.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 29.0 ms | 29.0 ms | -0.1% | [-3.4%, +7.3%] | - | ~ | device 0.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 58.5 ms | 58.9 ms | +0.6% | [-1.3%, +1.9%] | - | ~ | device 0.7x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 15.5 ms | 15.4 ms | -0.2% | [-0.9%, +1.0%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 19.8 ms | 19.9 ms | +0.4% | [-2.7%, +3.2%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 39.6 ms | 39.6 ms | -0.0% | [-1.6%, +1.0%] | - | ~ | device 0.6x faster than the host |
| `Render/csg/firstHit-camera/float/hostdevice` | 2.66 ms | 485 µs | -81.8% | [-82.1%, -81.6%] | - | faster | device 2.1x faster than the host (was 0.5x); checksum differs (303539.7443 -> 303212.6255) |
| `Render/union64/firstHit-camera/float/hostdevice` | - | - | - | - | - | new |  |
| `Render/mixed/firstHit-camera/float/hostdevice` | - | - | - | - | - | new |  |
| `Render/mesh/firstHit-camera/float/hostdevice` | 407 µs | 407 µs | +0.0% | [-0.3%, +1.1%] | - | ~ | device 0.4x faster than the host (was 0.5x) |
| `Render/csg/firstHit-camera/double/hostdevice` | 4.27 ms | 742 µs | -82.6% | [-82.7%, -82.4%] | - | faster | device 1.4x faster than the host (was 0.4x); checksum differs (303530.7417 -> 303206.4785) |
| `Render/union64/firstHit-camera/double/hostdevice` | - | - | - | - | - | new |  |
| `Render/mixed/firstHit-camera/double/hostdevice` | - | - | - | - | - | new |  |
| `Render/mesh/firstHit-camera/double/hostdevice` | 1.26 ms | 1.26 ms | +0.1% | [-1.0%, +1.1%] | - | ~ | device 0.2x faster than the host (was 0.1x) |
| `Tape/csg/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Tape/smooth/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Tape/union64/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Tape/csg/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `Tape/smooth/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `Tape/union64/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |


The probes differ between the sides (each side ran its own): Performance/Render/main.cpp, Performance/Tape/main.cpp, Performance/Common/Inputs.hpp

<!-- END receipt cuda -->
