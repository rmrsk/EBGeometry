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
base: 2f03add
base-fingerprint: f67ceadc9829a008d72ad0d2c0b7154865b793e2
head: a267dec
head-fingerprint: 982609cd2764de3fe93d6b25231aae658499f04a
cases: 136
rounds: 2
reps: 3
date: 2026-10-28
-->

## Host comparison: yellow

**Accepted:** -

`a267dec` against `2f03add` (origin/dev), 2026-10-28.

**VERDICT: 3 minor regressions

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: 2f03add
base-fingerprint: f67ceadc9829a008d72ad0d2c0b7154865b793e2
head: a267dec
head-fingerprint: 982609cd2764de3fe93d6b25231aae658499f04a
cases: 162
rounds: 2
reps: 3
date: 2026-10-28
-->

## Device comparison (CUDA): red

**Accepted:** -

`a267dec` against `2f03add` (origin/dev), 2026-10-28.

**VERDICT: host and device answers differ in 6 cases

<!-- END receipt cuda -->
