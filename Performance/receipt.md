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
base: efd0d95
base-fingerprint: 8f5012d6d47a6f34df899e1edd2cefbacb28ba5c
head: c16d132
head-fingerprint: 4473d6178a01cc7c1bc00dbbcf1c64262b94c98f
cases: 152
rounds: 2
reps: 3
date: 2026-10-30
-->

## Host comparison: yellow

**Accepted:** Accept it as side-effect of unfortunate compiler inlining.

`c16d132` against `efd0d95` (origin/dev), 2026-10-30.

**VERDICT: 2 minor regressions

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: efd0d95
base-fingerprint: 8f5012d6d47a6f34df899e1edd2cefbacb28ba5c
head: c16d132
head-fingerprint: 4473d6178a01cc7c1bc00dbbcf1c64262b94c98f
cases: 218
rounds: 2
reps: 3
date: 2026-10-30
-->

## Device comparison (CUDA): red

**Accepted:** Just accept it.

`c16d132` against `efd0d95` (origin/dev), 2026-10-30.

**VERDICT: host and device answers differ in 6 cases

<!-- END receipt cuda -->
