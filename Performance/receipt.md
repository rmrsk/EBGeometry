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
verdict: red
base: e65d2ef
base-fingerprint: 7cbe8dbd0203e980bb20b3345bd0f1e8985d5f47
head: 717bdac
head-fingerprint: a12d8746c21d02e1b2f4ee2d77aea8626442f95c
cases: 138
rounds: 2
reps: 3
date: 2026-10-28
-->

## Host comparison: red

**Accepted:** Accepted. The flagged Tape/csg-handwritten case is noise (10 rounds: −0.8%); TriMeshSDF at width 8 is 3% slower on random queries, the cost of signing the winner through an eight-wide group; width 4, the default, is 3–6% faster.

`717bdac` against `e65d2ef` (origin/dev), 2026-10-28.

**VERDICT: MAJOR regression in 1 case

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: e65d2ef
base-fingerprint: 7cbe8dbd0203e980bb20b3345bd0f1e8985d5f47
head: 717bdac
head-fingerprint: a12d8746c21d02e1b2f4ee2d77aea8626442f95c
cases: 164
rounds: 2
reps: 3
date: 2026-10-28
-->

## Device comparison (CUDA): red

**Accepted:** Accepted. Device code is unchanged (ptxas identical to dev); the 7 host/device disagreements are dev's own grazing float rays and 4 Render pixels.

`717bdac` against `e65d2ef` (origin/dev), 2026-10-28.

**VERDICT: host and device answers differ in 7 cases

<!-- END receipt cuda -->
