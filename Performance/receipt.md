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
base: 852aad3
base-fingerprint: 1bdd8c21d545ff6258ed0842567011e018b95043
head: 3d32b66
head-fingerprint: 25cab000b63bfba9cf3bee31a143f851ec6ab9c2
cases: 118
rounds: 3
reps: 5
date: 2026-10-22
-->

## Host comparison: yellow

**Accepted:** Algorithm changed

`3d32b66` against `852aad3` (origin/claude/renderer-examples), 2026-10-22.

**VERDICT: no regression, but 4 checksums changed: check those answers (see Flagged above)**

Performance comparison: 0 major, 0 minor, 5 faster, 0 differ, 0 lost, 118 cases

- base: 852aad3 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 3d32b66 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/build/float/host` | 55.1 ms | 55.3 ms | +0.3% | [-1.0%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 49.2 ms | 49.3 ms | +0.2% | [-0.7%, +1.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 65.0 ms | 65.2 ms | +0.4% | [-0.6%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.4 ms | 54.5 ms | +0.2% | [-1.3%, +37.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 61.1 ms | 60.7 ms | -0.7% | [-1.6%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 71.5 ms | 71.5 ms | +0.0% | [-1.6%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.1 ms | 74.8 ms | +0.9% | [-0.7%, +2.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 81.3 ms | 80.5 ms | -1.0% | [-20.4%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 67.5 ms | 67.3 ms | -0.3% | [-0.7%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 98.0 ms | 98.2 ms | +0.2% | [-1.0%, +1.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 82.5 ms | 82.8 ms | +0.4% | [-0.6%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 110 ms | 111 ms | +0.8% | [-1.6%, +0.9%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 82.0 ms | 82.1 ms | +0.0% | [-0.6%, +0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 242 ms | 244 ms | +0.6% | [+0.3%, +0.8%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 114 ms | 113 ms | -0.3% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 283 ms | 283 ms | -0.2% | [-2.5%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.7 ms | 70.5 ms | -0.2% | [-0.6%, +0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 213 ms | 212 ms | -0.3% | [-1.2%, +1.6%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.9 ms | 96.5 ms | -0.5% | [-0.8%, -0.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 30.4 ms | 30.3 ms | -0.3% | [-1.9%, +0.8%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 765 ms | 767 ms | +0.3% | [-0.4%, +0.9%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 119 ms | +0.6% | [-0.3%, +0.8%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 44.6 ms | 43.7 ms | -2.0% | [-3.4%, +1.0%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 869 ms | 864 ms | -0.5% | [-1.7%, +0.7%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/build/float/host` | 68.4 ms | 68.6 ms | +0.2% | [-1.0%, +0.6%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 150 ms | 150 ms | +0.1% | [-0.2%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 370 ms | 370 ms | -0.0% | [-1.0%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 78.2 ms | 78.2 ms | +0.0% | [-0.6%, +0.8%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.64 ms | 6.61 ms | -0.4% | [-0.6%, +1.7%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/float/host` | 146 ms | 146 ms | +0.2% | [-0.4%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/float/host` | 338 ms | 338 ms | +0.0% | [-0.2%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/float/host` | 184 ms | 183 ms | -0.1% | [-0.5%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 81.8 ms | 81.2 ms | -0.7% | [-1.5%, +1.3%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 168 ms | 168 ms | +0.3% | [-1.1%, +1.1%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 412 ms | 412 ms | -0.1% | [-4.4%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 88.3 ms | 89.1 ms | +0.9% | [-1.2%, +39.8%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.96 ms | 7.08 ms | +1.6% | [-0.3%, +4.4%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/double/host` | 160 ms | 159 ms | -0.5% | [-0.7%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/double/host` | 362 ms | 364 ms | +0.6% | [-17.0%, +4.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/double/host` | 192 ms | 191 ms | -0.4% | [-0.6%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 11.7 ms | 11.3 ms | -3.1% | [-20.8%, +2.3%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 125 ms | 125 ms | +0.1% | [-0.2%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 536 ms | 536 ms | -0.0% | [-0.5%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 153 ms | 153 ms | +0.0% | [-0.3%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 863 ms | 864 ms | +0.1% | [-0.2%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/float/host` | 121 ms | 121 ms | -0.0% | [-0.3%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/host` | 512 ms | 512 ms | +0.2% | [+0.1%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/float/host` | 361 ms | 362 ms | +0.1% | [-0.3%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 14.9 ms | 14.9 ms | +0.3% | [-0.2%, +5.9%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 129 ms | 129 ms | -0.0% | [-0.7%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 561 ms | 562 ms | +0.1% | [-0.5%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 156 ms | 157 ms | +0.8% | [+0.3%, +2.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 958 ms | 959 ms | +0.1% | [-0.3%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/double/host` | 122 ms | 122 ms | +0.0% | [-1.2%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/host` | 520 ms | 520 ms | -0.1% | [-0.7%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/double/host` | 368 ms | 367 ms | -0.1% | [-0.3%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 67.8 ms | 67.5 ms | -0.4% | [-0.9%, +0.3%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 112 ms | 112 ms | -0.1% | [-0.9%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 269 ms | 268 ms | -0.2% | [-0.4%, +0.9%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 112 ms | 112 ms | -0.5% | [-0.9%, -0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 106 ms | 106 ms | +0.1% | [-0.5%, +0.7%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/float/host` | 109 ms | 109 ms | -0.3% | [-2.0%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/float/host` | 237 ms | 239 ms | +0.7% | [-1.3%, +1.0%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/float/host` | 193 ms | 192 ms | -0.2% | [-0.5%, +0.0%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 80.5 ms | 80.5 ms | -0.1% | [-1.1%, +0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 127 ms | 128 ms | +0.9% | [-1.2%, +1.6%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 305 ms | 306 ms | +0.2% | [-0.7%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 129 ms | 128 ms | -0.6% | [-1.2%, -0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 124 ms | 124 ms | +0.1% | [-0.2%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/double/host` | 119 ms | 119 ms | -0.4% | [-1.1%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/double/host` | 256 ms | 257 ms | +0.1% | [-0.4%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/double/host` | 201 ms | 200 ms | -0.5% | [-0.6%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 9.01 ms | 8.82 ms | -2.0% | [-15.5%, +2.3%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 143 ms | 144 ms | +0.3% | [-4.1%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 401 ms | 400 ms | -0.1% | [-0.5%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 258 ms | 256 ms | -0.7% | [-2.1%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 334 ms | 334 ms | -0.2% | [-0.4%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/float/host` | 148 ms | 147 ms | -0.4% | [-4.3%, +4.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/host` | 389 ms | 378 ms | -2.8% | [-3.1%, -0.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/float/host` | 350 ms | 351 ms | +0.3% | [-0.4%, +2.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 12.5 ms | 12.3 ms | -1.1% | [-3.2%, +0.0%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 147 ms | 147 ms | +0.1% | [-3.8%, +4.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 423 ms | 423 ms | -0.2% | [-3.4%, +2.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 294 ms | 291 ms | -1.0% | [-2.2%, +2.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 376 ms | 376 ms | -0.1% | [-0.7%, +1.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/double/host` | 149 ms | 143 ms | -4.2% | [-4.4%, +0.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/host` | 388 ms | 380 ms | -2.1% | [-3.3%, +3.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/double/host` | 349 ms | 348 ms | -0.3% | [-2.7%, +2.7%] | - | ~ |  |
| `Render/csg/firstHit/float/host` | 18.2 ms | 12.7 ms | -30.3% | [-31.1%, -29.8%] | +0.0% | faster | checksum differs (303538.3704 -> 303212.6064) |
| `Render/union64/firstHit/float/host` | - | - | - | - | - | new |  |
| `Render/mixed/firstHit/float/host` | - | - | - | - | - | new |  |
| `Render/mesh/firstHit/float/host` | 1.71 ms | 1.69 ms | -1.0% | [-1.8%, -0.6%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 25.7 ms | 15.6 ms | -39.4% | [-43.4%, -39.1%] | +0.0% | faster | checksum differs (1.288592882e+14 -> 1.293817267e+14) |
| `Render/csg/firstHit/double/host` | 20.3 ms | 13.5 ms | -33.4% | [-41.0%, -27.6%] | +0.0% | faster | checksum differs (303527.4187 -> 303206.4785) |
| `Render/union64/firstHit/double/host` | - | - | - | - | - | new |  |
| `Render/mixed/firstHit/double/host` | - | - | - | - | - | new |  |
| `Render/mesh/firstHit/double/host` | 1.80 ms | 1.73 ms | -3.7% | [-6.2%, -0.5%] | +0.0% | faster |  |
| `Render/csg/frame/double/host` | 27.5 ms | 16.3 ms | -40.7% | [-41.1%, -37.6%] | +0.0% | faster | checksum differs (1.288588646e+14 -> 1.293817097e+14) |
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


The probes differ between the sides (each side ran its own): Performance/Render/main.cpp, Performance/Tape/main.cpp, Performance/Common/Inputs.hpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: 852aad3
base-fingerprint: 1bdd8c21d545ff6258ed0842567011e018b95043
head: 3d32b66
head-fingerprint: 25cab000b63bfba9cf3bee31a143f851ec6ab9c2
cases: 88
rounds: 3
reps: 5
date: 2026-10-22
-->

## Device comparison (CUDA): yellow

**Accepted:** Algorithm changed

`3d32b66` against `852aad3` (origin/claude/renderer-examples), 2026-10-22.

**VERDICT: no regression, but 2 checksums changed: check those answers (see Flagged above)**

Performance comparison: 0 major, 0 minor, 2 faster, 0 differ, 0 lost, 88 cases

- base: 852aad3 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 3d32b66 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/query-morton/float/hostdevice` | 419 µs | 415 µs | -0.8% | [-1.5%, -0.2%] | - | ~ | device 13.0x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 937 µs | 935 µs | -0.2% | [-0.5%, +1.3%] | - | ~ | device 8.5x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.41 ms | 4.40 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.56 ms | 4.54 ms | -0.4% | [-0.9%, +0.1%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 697 µs | 693 µs | -0.5% | [-1.8%, -0.2%] | - | ~ | device 10.5x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.60 ms | 1.59 ms | -0.4% | [-0.6%, +0.3%] | - | ~ | device 7.0x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.62 ms | 6.64 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 6.87 ms | 6.87 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 1.12 ms | 1.12 ms | +0.4% | [-0.4%, +0.7%] | - | ~ | device 11.3x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.69 ms | 2.71 ms | +0.9% | [-1.2%, +2.8%] | - | ~ | device 6.4x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.28 ms | 9.28 ms | -0.0% | [-0.8%, +0.4%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 9.88 ms | 9.88 ms | -0.0% | [-0.1%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 9.20 ms | 9.15 ms | -0.5% | [-5.5%, +7.6%] | - | ~ | device 10.3x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 21.9 ms | 21.8 ms | -0.5% | [-2.1%, +2.7%] | - | ~ | device 5.5x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 141 ms | 141 ms | +0.4% | [-0.8%, +1.2%] | - | ~ | device 0.7x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 263 ms | 262 ms | -0.6% | [-2.4%, +2.8%] | - | ~ | device 0.5x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 29.0 ms | 28.8 ms | -0.5% | [-1.8%, +0.4%] | - | ~ | device 3.4x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 68.3 ms | 68.3 ms | +0.0% | [-0.9%, +0.4%] | - | ~ | device 2.1x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 139 ms | 136 ms | -1.8% | [-4.1%, +2.7%] | - | ~ | device 0.7x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 229 ms | 229 ms | +0.0% | [-3.0%, +3.4%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 783 µs | 782 µs | -0.1% | [-0.9%, +0.6%] | - | ~ | device 10.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 3.09 ms | 3.12 ms | +1.1% | [-0.2%, +1.3%] | - | ~ | device 4.1x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.16 ms | 1.17 ms | +0.2% | [-0.8%, +0.9%] | - | ~ | device 8.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 11.2 ms | 11.2 ms | +0.2% | [-0.3%, +0.6%] | - | ~ | device 2.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 6.62 ms | 6.67 ms | +0.7% | [-0.2%, +2.4%] | - | ~ | device 2.5x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 6.82 ms | 6.82 ms | +0.1% | [-2.8%, +2.1%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.65 ms | 7.66 ms | +0.1% | [-1.6%, +2.9%] | - | ~ | device 1.8x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.2 ms | 11.8 ms | -10.3% | [-10.4%, +0.2%] | - | ~ | device 0.9x faster than the host (was 0.8x) |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 25.6 ms | 25.5 ms | -0.3% | [-0.7%, +0.0%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 14.7 ms | 14.8 ms | +0.6% | [-0.3%, +1.0%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 2.05 ms | 2.04 ms | -0.4% | [-1.2%, +0.5%] | - | ~ | device 5.0x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 5.16 ms | 5.19 ms | +0.7% | [-1.4%, +1.5%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 859 µs | 859 µs | +0.1% | [-0.8%, +0.6%] | - | ~ | device 10.0x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 11.7 ms | 11.8 ms | +0.7% | [-1.1%, +1.9%] | - | ~ | device 2.8x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 7.36 ms | 7.39 ms | +0.4% | [-1.2%, +2.3%] | - | ~ | device 3.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.42 ms | 8.31 ms | -1.3% | [-17.0%, +22.9%] | - | ~ | device 1.3x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 16.1 ms | 13.9 ms | -14.1% | [-16.8%, +4.6%] | - | ~ | device 1.2x faster than the host (was 1.0x) |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 3.38 ms | 3.38 ms | -0.1% | [-1.2%, +1.3%] | - | ~ | device 2.6x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 24.8 ms | 24.9 ms | +0.4% | [+0.3%, +0.6%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 16.7 ms | 16.7 ms | +0.3% | [-0.9%, +2.6%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.49 ms | 1.48 ms | -0.6% | [-0.7%, +0.8%] | - | ~ | device 8.5x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 6.42 ms | 6.44 ms | +0.3% | [-1.0%, +1.2%] | - | ~ | device 2.9x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 1.36 ms | 1.37 ms | +0.3% | [-12.6%, +12.8%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 5.61 ms | 5.63 ms | +0.4% | [-1.2%, +1.1%] | - | ~ | device 3.3x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 7.55 ms | 7.54 ms | -0.2% | [-0.8%, +0.5%] | - | ~ | device 2.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 13.2 ms | 12.0 ms | -9.2% | [-12.5%, +12.5%] | - | ~ | device 1.1x faster than the host (was 1.0x) |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 15.5 ms | 15.6 ms | +0.4% | [-4.4%, +2.6%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 6.45 ms | 6.46 ms | +0.1% | [-1.2%, +1.9%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 15.4 ms | 15.4 ms | -0.1% | [-1.1%, +0.8%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 18.4 ms | 18.5 ms | +0.4% | [-1.7%, +0.6%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 6.32 ms | 6.46 ms | +2.3% | [-0.1%, +2.4%] | - | ~ | device 2.7x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 18.2 ms | 18.2 ms | -0.2% | [-0.5%, +0.5%] | - | ~ | device 1.8x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.91 ms | 1.91 ms | -0.1% | [-0.4%, +1.2%] | - | ~ | device 6.1x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 5.79 ms | 5.79 ms | +0.0% | [-2.8%, +1.1%] | - | ~ | device 4.7x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 14.8 ms | 14.8 ms | -0.3% | [-1.6%, +1.7%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 29.0 ms | 29.0 ms | -0.3% | [-9.2%, +0.8%] | - | ~ | device 0.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 59.1 ms | 58.7 ms | -0.8% | [-1.6%, +1.9%] | - | ~ | device 0.7x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 15.4 ms | 15.4 ms | +0.2% | [-1.0%, +0.8%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 19.9 ms | 19.9 ms | +0.5% | [-1.7%, +2.3%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 40.0 ms | 40.0 ms | +0.1% | [-1.2%, +0.6%] | - | ~ | device 0.6x faster than the host |
| `Render/csg/firstHit-camera/float/hostdevice` | 2.68 ms | 484 µs | -81.9% | [-82.0%, -81.8%] | - | faster | device 2.0x faster than the host (was 0.5x); checksum differs (303539.7443 -> 303212.6255) |
| `Render/union64/firstHit-camera/float/hostdevice` | - | - | - | - | - | new |  |
| `Render/mixed/firstHit-camera/float/hostdevice` | - | - | - | - | - | new |  |
| `Render/mesh/firstHit-camera/float/hostdevice` | 408 µs | 407 µs | -0.2% | [-0.4%, +0.8%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `Render/csg/firstHit-camera/double/hostdevice` | 4.27 ms | 743 µs | -82.6% | [-82.7%, -82.5%] | - | faster | device 1.5x faster than the host (was 0.4x); checksum differs (303530.7417 -> 303206.4785) |
| `Render/union64/firstHit-camera/double/hostdevice` | - | - | - | - | - | new |  |
| `Render/mixed/firstHit-camera/double/hostdevice` | - | - | - | - | - | new |  |
| `Render/mesh/firstHit-camera/double/hostdevice` | 1.27 ms | 1.26 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
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


The probes differ between the sides (each side ran its own): Performance/Render/main.cpp, Performance/Tape/main.cpp, Performance/Common/Inputs.hpp

<!-- END receipt cuda -->
