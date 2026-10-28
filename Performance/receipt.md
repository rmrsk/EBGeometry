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
head: 3544549
head-fingerprint: 704951f9511e922b0cc01e21a2a8cd31b91101d2
cases: 136
rounds: 2
reps: 3
date: 2026-10-28
-->

## Host comparison: yellow

**Accepted:** Accept receipt as side effects of new rendering engine.

`3544549` against `2f03add` (origin/dev), 2026-10-28.

**VERDICT: 3 minor regressions

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: 2f03add
base-fingerprint: f67ceadc9829a008d72ad0d2c0b7154865b793e2
head: 3544549
head-fingerprint: 704951f9511e922b0cc01e21a2a8cd31b91101d2
cases: 162
rounds: 2
reps: 3
date: 2026-10-28
-->

## Device comparison (CUDA): red

**Accepted:** Accept receipt as side effects of new rendering engine.

`3544549` against `2f03add` (origin/dev), 2026-10-28.

**VERDICT: host and device answers differ in 6 cases

<!-- END receipt cuda -->
