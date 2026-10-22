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
verdict: red
accepted: The two flagged builds (TriMeshSDF w8 float, PointCloudBVH sphere double) are allocator luck in how the harness times builds, not BVH-8: with glibc's mmap threshold pinned, both sides build equally fast. A harness fix follows in a separate PR. Accepted by the maintainer.
base: 5e4b1da
base-fingerprint: 0d01808f51ae690edc0234181c00c581e14fd6fd
head: 7544f81
head-fingerprint: 866e6a5180c53f362ab3ae9e222322a43f91e901
cases: 88
rounds: 3
reps: 5
date: 2026-10-22
-->

## Host comparison: red

`7544f81` against `5e4b1da` (origin/dev), 2026-10-22.

**VERDICT: MAJOR regression in 2 cases

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 2
side: cuda
verdict: red
accepted: nvcc 12.5 schedules the unchanged leaf-scan loads less well after BVH-8's node changes: the same loads and sectors, but 27% more stalls waiting for memory (Nsight Compute). Wide leaf loads follow in a separate PR. Accepted by the maintainer.
base: 5e4b1da
base-fingerprint: 0d01808f51ae690edc0234181c00c581e14fd6fd
head: 7544f81
head-fingerprint: 866e6a5180c53f362ab3ae9e222322a43f91e901
cases: 60
rounds: 3
reps: 5
date: 2026-10-22
-->

## Device comparison (CUDA): red

`7544f81` against `5e4b1da` (origin/dev), 2026-10-22.

**VERDICT: MAJOR regression in 3 cases

<!-- END receipt cuda -->
