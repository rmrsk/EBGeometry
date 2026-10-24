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
base: 58a78db
base-fingerprint: e82f696417f53100ce7598a10c5f88662693de2f
head: 6392744
head-fingerprint: 204f1728277337f2caa9e89bde7a9437ccc24a5c
cases: 126
rounds: 3
reps: 5
date: 2026-10-24
-->

## Host comparison: yellow

**Accepted:** false regression

`6392744` against `58a78db` (origin/dev), 2026-10-24.

**VERDICT: 1 minor regression

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: 58a78db
base-fingerprint: e82f696417f53100ce7598a10c5f88662693de2f
head: 6392744
head-fingerprint: 204f1728277337f2caa9e89bde7a9437ccc24a5c
cases: 98
rounds: 3
reps: 5
date: 2026-10-24
-->

## Device comparison (CUDA): green

**Accepted:** -

`6392744` against `58a78db` (origin/dev), 2026-10-24.

**VERDICT: no regression (0 major, 0 minor across 98 cases)**

Performance comparison: 0 major, 0 minor, 1 faster, 0 differ, 0 lost, 98 cases

- base: 58a78db (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 6392744 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/query-morton/float/hostdevice` | 418 µs | 418 µs | +0.0% | [-1.1%, +0.4%] | - | ~ | device 13.2x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 935 µs | 937 µs | +0.3% | [-0.5%, +1.0%] | - | ~ | device 8.6x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.40 ms | 4.44 ms | +0.8% | [+0.4%, +0.9%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.56 ms | 4.55 ms | -0.1% | [-1.2%, +0.1%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 690 µs | 693 µs | +0.3% | [+0.2%, +0.9%] | - | ~ | device 10.6x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.59 ms | 1.59 ms | +0.1% | [-0.3%, +0.2%] | - | ~ | device 7.0x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.63 ms | 6.61 ms | -0.3% | [-0.4%, +0.0%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 6.86 ms | 6.85 ms | -0.2% | [-0.3%, +0.6%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 1.11 ms | 1.11 ms | +0.2% | [-0.0%, +0.7%] | - | ~ | device 11.3x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.64 ms | 2.65 ms | +0.5% | [-0.1%, +1.1%] | - | ~ | device 6.6x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.30 ms | 9.29 ms | -0.1% | [-0.3%, +0.7%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 9.89 ms | 9.90 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | - | - | - | - | - | new | tape/fused 8.12x |
| `ClosestPrimitive/csg-tape/closestPrimitive/float/hostdevice` | - | - | - | - | - | new |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | - | - | - | - | - | new | tape/fused 1.60x |
| `ClosestPrimitive/drilled-mesh-tape/closestPrimitive/float/hostdevice` | - | - | - | - | - | new |  |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | - | - | - | - | - | new | tape/fused 0.95x |
| `ClosestPrimitive/csg-tape/closestPrimitive/double/hostdevice` | - | - | - | - | - | new |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | - | - | - | - | - | new | tape/fused 1.00x |
| `ClosestPrimitive/drilled-mesh-tape/closestPrimitive/double/hostdevice` | - | - | - | - | - | new |  |
| `Fused/csg/signedDistance/float/hostdevice` | 47.9 µs | 47.9 µs | +0.1% | [-0.5%, +0.5%] | - | ~ | device 23.3x faster than the host

<!-- END receipt cuda -->
