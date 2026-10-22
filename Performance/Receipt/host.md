<!--
SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>

SPDX-License-Identifier: GPL-3.0-or-later
-->

<!-- A performance receipt, written by Scripts/run-performance.sh compare --record and checked by CI
     (Performance/CheckReceipt.cmake). Commit it with the pull request; do not edit it, except to accept a
     yellow or red verdict: write the reason after "accepted:" below. See PerformanceTesting.rst. -->

<!--receipt
version: 1
side: host
verdict: green
accepted:
base: 000723d
base-fingerprint: 17359840f763a281b107aeb1032fe79ff33f801c
head: 35370b9
head-fingerprint: 7594f54a6030ee832c6bc7e172a06cb29d6be8fa
cases: 88
rounds: 3
reps: 5
date: 2026-10-22
-->

Performance receipt (host): `35370b9` against `000723d` (origin/dev), 2026-10-22.

**VERDICT: no regression (0 major, 0 minor across 88 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 88 cases

- base: 000723d (gcc 13.3.0, simd avx2, gpu host)
- head: 35370b9 (gcc 13.3.0, simd avx2, gpu host)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/build/float/host` | 54.3 ms | 53.6 ms | -1.3% | [-2.9%, +4.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 52.4 ms | 53.3 ms | +1.6% | [-3.3%, +3.7%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 68.2 ms | 68.4 ms | +0.3% | [-1.0%, +2.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 60.3 ms | 59.0 ms | -2.2% | [-9.6%, +2.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 61.1 ms | 61.9 ms | +1.2% | [-1.9%, +4.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 75.6 ms | 75.2 ms | -0.5% | [-5.8%, +3.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 72.8 ms | 73.2 ms | +0.7% | [-4.9%, +2.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 89.2 ms | 87.3 ms | -2.1% | [-24.1%, +6.5%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 65.2 ms | 64.7 ms | -0.8% | [-5.3%, +3.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 117 ms | 117 ms | +0.3% | [-16.5%, +11.8%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 82.3 ms | 83.6 ms | +1.6% | [-4.7%, +4.8%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 129 ms | 123 ms | -4.3% | [-18.7%, +39.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 71.5 ms | 73.5 ms | +2.8% | [-4.3%, +7.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 255 ms | 249 ms | -2.2% | [-4.3%, +11.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 89.0 ms | 87.2 ms | -2.1% | [-4.7%, +3.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 300 ms | 297 ms | -0.8% | [-4.6%, +2.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 54.2 ms | 53.8 ms | -0.8% | [-3.1%, +3.8%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 222 ms | 219 ms | -1.2% | [-4.9%, +8.5%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 91.3 ms | 91.2 ms | -0.1% | [-3.4%, +3.8%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 37.2 ms | 37.5 ms | +1.0% | [-0.8%, +18.6%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 835 ms | 844 ms | +1.1% | [-17.2%, +10.4%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 100 ms | 102 ms | +1.3% | [-2.5%, +4.6%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 77.2 ms | 71.6 ms | -7.2% | [-18.5%, +33.8%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 895 ms | 906 ms | +1.3% | [-17.0%, +9.4%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/build/float/host` | 69.7 ms | 69.4 ms | -0.4% | [-0.8%, +0.4%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 152 ms | 153 ms | +0.8% | [-1.3%, +1.9%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 373 ms | 372 ms | -0.3% | [-1.6%, +1.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 81.0 ms | 81.7 ms | +0.8% | [-0.1%, +2.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.87 ms | 7.02 ms | +2.2% | [-1.2%, +2.8%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/float/host` | 150 ms | 150 ms | -0.2% | [-1.4%, +2.7%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/float/host` | 340 ms | 341 ms | +0.4% | [-5.9%, +10.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/float/host` | 183 ms | 182 ms | -0.4% | [-3.6%, +3.4%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 83.5 ms | 83.7 ms | +0.2% | [-6.9%, +11.3%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 163 ms | 164 ms | +0.3% | [-16.6%, +19.4%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 435 ms | 417 ms | -4.1% | [-6.7%, +27.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 92.3 ms | 92.4 ms | +0.1% | [-23.2%, +39.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.21 ms | 7.21 ms | -0.0% | [-24.6%, +31.1%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/double/host` | 156 ms | 155 ms | -0.2% | [-33.2%, +56.3%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/double/host` | 364 ms | 366 ms | +0.6% | [-5.5%, +4.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/double/host` | 193 ms | 195 ms | +1.0% | [-1.0%, +2.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 11.5 ms | 11.6 ms | +0.1% | [-1.4%, +1.9%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 124 ms | 124 ms | +0.2% | [-8.0%, +9.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 536 ms | 536 ms | -0.1% | [-3.8%, +4.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 154 ms | 154 ms | -0.3% | [-2.4%, +4.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 865 ms | 877 ms | +1.3% | [-3.7%, +5.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/float/host` | 125 ms | 121 ms | -2.7% | [-6.6%, +6.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/host` | 512 ms | 518 ms | +1.1% | [-3.3%, +4.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/float/host` | 361 ms | 360 ms | -0.1% | [-2.6%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 9.05 ms | 9.15 ms | +1.1% | [-0.5%, +7.3%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 129 ms | 128 ms | -0.3% | [-1.7%, +6.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 562 ms | 563 ms | +0.1% | [-0.6%, +1.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 157 ms | 156 ms | -0.8% | [-1.4%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 950 ms | 955 ms | +0.6% | [-4.3%, +4.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/double/host` | 122 ms | 122 ms | +0.0% | [-11.9%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/host` | 517 ms | 518 ms | +0.1% | [-0.6%, +2.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/double/host` | 364 ms | 365 ms | +0.2% | [-0.1%, +5.1%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 65.1 ms | 65.1 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 112 ms | 112 ms | +0.4% | [+0.0%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 271 ms | 271 ms | +0.1% | [-0.7%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 118 ms | 118 ms | +0.3% | [-0.3%, +0.7%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 111 ms | 110 ms | -0.4% | [-1.3%, +0.0%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/float/host` | 110 ms | 110 ms | +0.0% | [-0.2%, +1.0%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/float/host` | 240 ms | 240 ms | -0.2% | [-1.2%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/float/host` | 190 ms | 190 ms | -0.0% | [-0.2%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 70.0 ms | 70.0 ms | +0.0% | [-0.2%, +1.3%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 122 ms | 122 ms | +0.5% | [-17.2%, +1.2%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 313 ms | 313 ms | +0.1% | [-0.1%, +8.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 134 ms | 135 ms | +0.3% | [-0.9%, +1.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 129 ms | 130 ms | +0.4% | [-0.9%, +0.9%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/double/host` | 114 ms | 114 ms | -0.0% | [-0.9%, +1.0%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/double/host` | 258 ms | 258 ms | +0.1% | [-0.2%, +1.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/double/host` | 202 ms | 202 ms | -0.0% | [-1.9%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 8.80 ms | 8.87 ms | +0.8% | [-0.5%, +1.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 141 ms | 141 ms | -0.0% | [-0.6%, +4.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 407 ms | 409 ms | +0.5% | [-6.0%, +5.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 253 ms | 255 ms | +0.8% | [-6.7%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 335 ms | 334 ms | -0.1% | [-2.6%, +5.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/float/host` | 146 ms | 146 ms | -0.1% | [-6.3%, +4.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/host` | 384 ms | 385 ms | +0.1% | [-3.7%, +4.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/float/host` | 353 ms | 346 ms | -2.0% | [-3.4%, +3.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 6.45 ms | 6.42 ms | -0.4% | [-3.6%, +3.6%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 145 ms | 149 ms | +2.9% | [-2.2%, +4.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 438 ms | 433 ms | -1.2% | [-3.3%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 290 ms | 290 ms | -0.2% | [-3.6%, +7.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 380 ms | 372 ms | -2.0% | [-3.0%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/double/host` | 148 ms | 149 ms | +0.6% | [-0.2%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/host` | 378 ms | 374 ms | -1.1% | [-3.4%, +2.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/double/host` | 346 ms | 354 ms | +2.2% | [-6.9%, +7.7%] | - | ~ |  |

The baseline has no Performance/ folder; it was measured with the candidate's probes.
