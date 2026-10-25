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
base: 4d0d0ac
base-fingerprint: b994d5a15d03f7c986c18ff5975e90555d814775
head: f4b1c46
head-fingerprint: 0ee78d5210969340a6894041b440fce47fd0195d
cases: 130
rounds: 3
reps: 5
date: 2026-10-25
-->

## Host comparison: yellow

**Accepted:** -

`f4b1c46` against `4d0d0ac` (origin/dev), 2026-10-25.

**VERDICT: 3 minor regressions

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: 4d0d0ac
base-fingerprint: b994d5a15d03f7c986c18ff5975e90555d814775
head: f4b1c46
head-fingerprint: 0ee78d5210969340a6894041b440fce47fd0195d
cases: 108
rounds: 3
reps: 5
date: 2026-10-25
-->

## Device comparison (CUDA): yellow

**Accepted:** -

`f4b1c46` against `4d0d0ac` (origin/dev), 2026-10-25.

**VERDICT: 7 minor regressions

<!-- END receipt cuda -->
