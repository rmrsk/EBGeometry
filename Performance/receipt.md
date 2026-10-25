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
head: 1d92860
head-fingerprint: d0c3f5bcd0b6ba766166e3788efb7c42a6e6bdd9
cases: 130
rounds: 3
reps: 5
date: 2026-10-25
-->

## Host comparison: yellow

**Accepted:** -

`1d92860` against `4d0d0ac` (origin/dev), 2026-10-25.

**VERDICT: 1 minor regression

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: 4d0d0ac
base-fingerprint: b994d5a15d03f7c986c18ff5975e90555d814775
head: 1d92860
head-fingerprint: d0c3f5bcd0b6ba766166e3788efb7c42a6e6bdd9
cases: 108
rounds: 3
reps: 5
date: 2026-10-25
-->

## Device comparison (CUDA): yellow

**Accepted:** -

`1d92860` against `4d0d0ac` (origin/dev), 2026-10-25.

**VERDICT: 6 minor regressions

<!-- END receipt cuda -->
