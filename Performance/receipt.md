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
verdict: green
base: ac53392
base-fingerprint: fbf75b59076b6fbcc20ee17ec308dd941ac09f5d
head: 3ec3ff0
head-fingerprint: ee5de35b8fe17a5503f71bbbcab6df079f6783f4
cases: 108
rounds: 3
reps: 5
date: 2026-10-23
-->

## Host comparison: green

**Accepted:** -

`3ec3ff0` against `ac53392` (origin/dev), 2026-10-23.

**VERDICT: no regression (0 major, 0 minor across 108 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 108 cases

- base: ac53392 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 3ec3ff0 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/build/float/host` | 56.9 ms | 56.9 ms | -0.0% | [-2.2%, +1.6%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 50.5 ms | 50.8 ms | +0.7% | [-0.6%, +6.8%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 66.0 ms | 66.0 ms | -0.1% | [-0.6%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 55.4 ms | 55.5 ms | +0.1% | [-2.6%, +1.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 61.2 ms | 61.2 ms | +0.0% | [-2.4%, +3.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 73.9 ms | 72.6 ms | -1.8% | [-2.4%, +2.7%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 75.2 ms | 75.9 ms | +1.0% | [-0.4%, +1.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 84.1 ms | 83.5 ms | -0.8% | [-1.8%, +1.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 67.6 ms | 67.7 ms | +0.1% | [-0.8%, +1.8%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 102 ms | 102 ms | +0.2% | [-0.7%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 84.8 ms | 85.0 ms | +0.3% | [-0.6%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 115 ms | 116 ms | +0.8% | [-2.0%, +2.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 84.0 ms | 83.9 ms | -0.0% | [-1.8%, +1.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 245 ms | 247 ms | +0.5% | [-1.2%, +1.8%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 115 ms | 115 ms | +0.1% | [-2.0%, +1.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 296 ms | 298 ms | +0.9% | [+0.1%, +2.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 71.8 ms | 72.3 ms | +0.7% | [+0.3%, +1.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 215 ms | 216 ms | +0.2% | [-2.4%, +2.0%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 98.2 ms | 98.1 ms | -0.0% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 31.8 ms | 31.9 ms | +0.2% | [-4.8%, +4.2%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 798 ms | 802 ms | +0.5% | [-2.4%, +1.8%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 121 ms | 120 ms | -0.5% | [-0.6%, +0.6%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 49.6 ms | 50.1 ms | +1.0% | [-1.6%, +4.0%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 929 ms | 928 ms | -0.1% | [-2.2%, +19.2%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/build/float/host` | 70.9 ms | 70.3 ms | -0.9% | [-3.9%, +2.5%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 154 ms | 154 ms | +0.1% | [-4.6%, +10.9%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 381 ms | 380 ms | -0.3% | [-2.2%, +3.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 79.6 ms | 80.0 ms | +0.4% | [-5.7%, +5.0%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.76 ms | 6.75 ms | -0.1% | [-4.5%, +5.6%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/float/host` | 150 ms | 151 ms | +0.3% | [-0.5%, +8.6%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/float/host` | 346 ms | 347 ms | +0.1% | [-2.9%, +2.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/float/host` | 187 ms | 187 ms | +0.0% | [-1.4%, +3.6%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 83.9 ms | 84.3 ms | +0.4% | [-1.2%, +1.9%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 174 ms | 175 ms | +0.6% | [-2.2%, +2.6%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 422 ms | 421 ms | -0.2% | [-3.8%, +2.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 91.6 ms | 92.3 ms | +0.8% | [-4.0%, +3.0%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.10 ms | 7.15 ms | +0.6% | [-4.8%, +2.3%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/double/host` | 164 ms | 163 ms | -0.5% | [-10.9%, +6.3%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/double/host` | 375 ms | 373 ms | -0.6% | [-3.6%, +3.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/double/host` | 197 ms | 197 ms | +0.4% | [-1.7%, +42.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 11.9 ms | 12.0 ms | +1.2% | [-2.0%, +4.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 129 ms | 127 ms | -1.3% | [-2.9%, +1.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 547 ms | 547 ms | -0.0% | [-2.0%, +3.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 156 ms | 156 ms | -0.0% | [-0.8%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 881 ms | 890 ms | +1.0% | [-0.5%, +2.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/float/host` | 122 ms | 123 ms | +1.1% | [-3.2%, +3.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/host` | 517 ms | 521 ms | +0.8% | [-2.7%, +3.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/float/host` | 366 ms | 370 ms | +1.1% | [-3.2%, +3.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.9 ms | 15.9 ms | +0.2% | [-0.9%, +2.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 130 ms | 133 ms | +2.5% | [-4.4%, +5.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 568 ms | 575 ms | +1.3% | [-1.3%, +2.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 162 ms | 162 ms | +0.2% | [-1.8%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 990 ms | 991 ms | +0.1% | [-12.2%, +4.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/double/host` | 125 ms | 126 ms | +0.5% | [-2.6%, +3.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/host` | 541 ms | 532 ms | -1.6% | [-2.7%, +2.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/double/host` | 375 ms | 376 ms | +0.3% | [-2.5%, +2.7%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 69.4 ms | 69.8 ms | +0.5% | [-1.2%, +1.3%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 116 ms | 115 ms | -0.4% | [-1.4%, +10.4%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 277 ms | 275 ms | -0.6% | [-1.9%, +0.7%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 115 ms | 115 ms | -0.2% | [-4.6%, +1.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 108 ms | 109 ms | +1.1% | [-0.8%, +1.4%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/float/host` | 111 ms | 111 ms | -0.1% | [-2.5%, +1.7%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/float/host` | 243 ms | 244 ms | +0.5% | [-2.3%, +2.5%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/float/host` | 196 ms | 196 ms | +0.3% | [+0.0%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 83.2 ms | 84.1 ms | +1.1% | [-2.1%, +2.2%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 132 ms | 133 ms | +0.4% | [-0.7%, +10.6%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 313 ms | 313 ms | +0.1% | [-4.6%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 134 ms | 133 ms | -0.7% | [-1.7%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 128 ms | 128 ms | -0.5% | [-1.8%, +1.9%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/double/host` | 123 ms | 124 ms | +0.8% | [-0.2%, +2.3%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/double/host` | 266 ms | 266 ms | +0.0% | [-8.7%, +5.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/double/host` | 205 ms | 204 ms | -0.2% | [-1.1%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 9.11 ms | 9.16 ms | +0.6% | [+0.3%, +1.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 147 ms | 147 ms | -0.2% | [-4.7%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 411 ms | 416 ms | +1.4% | [-1.6%, +3.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 285 ms | 260 ms | -8.7% | [-9.1%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 341 ms | 341 ms | -0.3% | [-1.8%, +4.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/float/host` | 145 ms | 151 ms | +4.1% | [-1.8%, +6.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/host` | 396 ms | 396 ms | -0.1% | [-4.7%, +4.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/float/host` | 356 ms | 356 ms | +0.1% | [-3.3%, +1.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 12.9 ms | 13.0 ms | +0.4% | [-0.6%, +1.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 150 ms | 154 ms | +2.5% | [-6.4%, +4.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 445 ms | 431 ms | -3.2% | [-3.5%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 296 ms | 300 ms | +1.5% | [-1.7%, +10.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 387 ms | 386 ms | -0.1% | [-2.1%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/double/host` | 146 ms | 145 ms | -0.2% | [-2.2%, +1.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/host` | 384 ms | 383 ms | -0.2% | [-1.7%, +3.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/double/host` | 354 ms | 355 ms | +0.2% | [-1.1%, +3.9%] | - | ~ |  |
| `Tape/csg/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Tape/csg-handwritten/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Tape/smooth/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Tape/union64/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Tape/sphere/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Tape/sphere/signedDistance-x16/float/host` | - | - | - | - | - | new |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | - | - | - | - | - | new |  |
| `Tape/chain16/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Tape/csg/signedDistance/double/host` | - | - | - | - | - | new |  |
| `Tape/csg-handwritten/signedDistance/double/host` | - | - | - | - | - | new |  |
| `Tape/smooth/signedDistance/double/host` | - | - | - | - | - | new |  |
| `Tape/union64/signedDistance/double/host` | - | - | - | - | - | new |  |
| `Tape/sphere/signedDistance/double/host` | - | - | - | - | - | new |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | - | - | - | - | - | new |  |
| `Tape/sphere/signedDistance-x16/double/host` | - | - | - | - | - | new |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | - | - | - | - | - | new |  |
| `Tape/chain16/signedDistance/double/host` | - | - | - | - | - | new |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | - | - | - | - | - | new |  |


The probes differ between the sides (each side ran its own): Performance/Tape/main.cpp, Performance/Common/Inputs.hpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: ac53392
base-fingerprint: fbf75b59076b6fbcc20ee17ec308dd941ac09f5d
head: 3ec3ff0
head-fingerprint: ee5de35b8fe17a5503f71bbbcab6df079f6783f4
cases: 80
rounds: 3
reps: 5
date: 2026-10-23
-->

## Device comparison (CUDA): green

**Accepted:** -

`3ec3ff0` against `ac53392` (origin/dev), 2026-10-23.

**VERDICT: no regression (0 major, 0 minor across 80 cases)**

Performance comparison: 0 major, 0 minor, 1 faster, 0 differ, 0 lost, 80 cases

- base: ac53392 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 3ec3ff0 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/query-morton/float/hostdevice` | 421 µs | 417 µs | -1.0% | [-1.6%, +0.2%] | - | ~ | device 13.0x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 945 µs | 938 µs | -0.8% | [-1.9%, +0.2%] | - | ~ | device 8.5x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 5.16 ms | 5.17 ms | +0.3% | [-0.7%, +1.5%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 5.28 ms | 5.28 ms | +0.1% | [-1.9%, +1.0%] | - | ~ | device 1.6x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 691 µs | 690 µs | -0.1% | [-1.1%, +1.1%] | - | ~ | device 10.7x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.61 ms | 1.64 ms | +1.8% | [-0.8%, +5.0%] | - | ~ | device 6.8x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 7.35 ms | 7.35 ms | +0.1% | [-0.1%, +0.5%] | - | ~ | device 1.0x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.62 ms | 7.61 ms | -0.1% | [-0.6%, +0.5%] | - | ~ | device 1.5x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 1.12 ms | 1.12 ms | +0.4% | [-0.9%, +6.3%] | - | ~ | device 11.3x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.82 ms | 2.83 ms | +0.5% | [-0.5%, +1.9%] | - | ~ | device 6.2x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 10.1 ms | 10.1 ms | -0.4% | [-0.7%, +0.8%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.7 ms | 10.8 ms | +1.4% | [-0.8%, +2.2%] | - | ~ | device 1.7x faster than the host |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 9.38 ms | 9.57 ms | +2.0% | [+0.6%, +4.2%] | - | ~ | device 9.9x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 21.3 ms | 20.8 ms | -2.3% | [-2.7%, +0.9%] | - | ~ | device 5.8x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 152 ms | 152 ms | -0.2% | [-0.7%, +1.5%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 285 ms | 285 ms | +0.0% | [-2.8%, +2.1%] | - | ~ | device 0.5x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 31.9 ms | 31.7 ms | -0.4% | [-1.0%, +1.2%] | - | ~ | device 3.1x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 74.9 ms | 75.0 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 1.9x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 150 ms | 149 ms | -0.5% | [-2.0%, +3.0%] | - | ~ | device 0.7x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 249 ms | 248 ms | -0.2% | [-2.6%, +3.9%] | - | ~ | device 0.6x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 787 µs | 785 µs | -0.2% | [-7.2%, +0.3%] | - | ~ | device 10.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 3.23 ms | 3.24 ms | +0.0% | [-1.6%, +2.7%] | - | ~ | device 3.9x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.16 ms | 1.17 ms | +0.1% | [-0.6%, +0.2%] | - | ~ | device 8.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 12.7 ms | 12.0 ms | -5.6% | [-5.9%, +1.6%] | - | ~ | device 2.3x faster than the host (was 2.2x) |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 7.21 ms | 7.24 ms | +0.5% | [+0.0%, +1.0%] | - | ~ | device 2.4x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 6.63 ms | 6.68 ms | +0.7% | [-15.5%, +22.0%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 8.48 ms | 8.45 ms | -0.3% | [-0.7%, +10.0%] | - | ~ | device 1.6x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 14.7 ms | 13.3 ms | -9.5% | [-19.0%, +26.4%] | - | ~ | device 0.8x faster than the host (was 0.7x) |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 27.8 ms | 28.0 ms | +1.1% | [-0.1%, +1.7%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 16.2 ms | 16.2 ms | +0.2% | [-0.9%, +1.7%] | - | ~ | device 1.1x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 2.05 ms | 2.02 ms | -1.0% | [-3.8%, -0.3%] | - | ~ | device 5.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 5.82 ms | 5.78 ms | -0.6% | [-1.1%, +0.8%] | - | ~ | device 2.7x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 862 µs | 858 µs | -0.5% | [-1.4%, +0.6%] | - | ~ | device 10.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 13.0 ms | 12.9 ms | -0.2% | [-4.2%, +2.3%] | - | ~ | device 2.7x faster than the host (was 2.6x) |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 7.89 ms | 8.01 ms | +1.4% | [-2.3%, +2.5%] | - | ~ | device 3.1x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 7.62 ms | 7.67 ms | +0.6% | [-1.8%, +25.0%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.0 ms | 15.0 ms | -0.5% | [-16.2%, -0.2%] | - | ~ | device 1.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 3.39 ms | 3.41 ms | +0.4% | [-0.7%, +19.0%] | - | ~ | device 2.6x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 27.2 ms | 27.1 ms | -0.6% | [-1.0%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 18.0 ms | 17.9 ms | -0.3% | [-0.6%, +1.3%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.49 ms | 1.49 ms | +0.1% | [-1.0%, +1.3%] | - | ~ | device 8.5x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 7.13 ms | 7.12 ms | -0.2% | [-0.3%, +0.0%] | - | ~ | device 2.6x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 1.37 ms | 1.52 ms | +11.1% | [-9.9%, +13.3%] | - | ~ | device 5.2x faster than the host (was 5.7x) |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 6.23 ms | 6.17 ms | -1.0% | [-4.9%, +0.5%] | - | ~ | device 3.0x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 8.33 ms | 8.33 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 2.3x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 14.6 ms | 12.0 ms | -17.3% | [-20.3%, +17.5%] | - | ~ | device 1.1x faster than the host (was 0.9x) |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 16.8 ms | 16.8 ms | +0.3% | [-3.7%, +1.1%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 7.22 ms | 7.14 ms | -1.1% | [-1.9%, +1.5%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 16.9 ms | 16.8 ms | -0.6% | [-2.8%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.1 ms | 20.0 ms | -0.6% | [-1.2%, +2.0%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 7.08 ms | 6.77 ms | -4.4% | [-5.7%, -2.7%] | - | faster | device 2.5x faster than the host (was 2.4x) |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 19.7 ms | 19.1 ms | -2.8% | [-3.4%, -2.3%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.92 ms | 1.93 ms | +0.2% | [-0.1%, +9.4%] | - | ~ | device 6.1x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 6.40 ms | 6.43 ms | +0.6% | [-5.1%, +6.8%] | - | ~ | device 4.2x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 15.9 ms | 16.1 ms | +1.0% | [-1.5%, +2.7%] | - | ~ | device 1.5x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 31.8 ms | 31.6 ms | -0.7% | [-3.4%, +2.4%] | - | ~ | device 0.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 65.0 ms | 65.0 ms | +0.1% | [-2.9%, +2.8%] | - | ~ | device 0.6x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 16.7 ms | 16.8 ms | +0.5% | [-0.6%, +0.9%] | - | ~ | device 0.7x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 21.8 ms | 21.5 ms | -1.6% | [-3.4%, +1.6%] | - | ~ | device 1.3x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 43.2 ms | 42.7 ms | -1.1% | [-1.7%, +1.3%] | - | ~ | device 0.6x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Tape/smooth/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Tape/union64/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Tape/sphere/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | - | - | - | - | - | new |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | - | - | - | - | - | new |  |
| `Tape/chain16/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Tape/csg/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `Tape/smooth/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `Tape/union64/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `Tape/sphere/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | - | - | - | - | - | new |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | - | - | - | - | - | new |  |
| `Tape/chain16/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |


The probes differ between the sides (each side ran its own): Performance/Tape/main.cpp, Performance/Common/Inputs.hpp

<!-- END receipt cuda -->
