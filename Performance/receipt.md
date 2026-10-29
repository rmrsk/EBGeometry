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
base: 5619c3e
base-fingerprint: 48c61b42ee97f3c8b09fc1ceffc49bc9ad4b0a98
head: f396ac7
head-fingerprint: 8f5012d6d47a6f34df899e1edd2cefbacb28ba5c
cases: 142
rounds: 2
reps: 3
date: 2026-10-29
-->

## Host comparison: red

**Accepted:** -

`f396ac7` against `5619c3e` (origin/dev), 2026-10-29.

**VERDICT: MAJOR regression in 2 cases

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: 5619c3e
base-fingerprint: 48c61b42ee97f3c8b09fc1ceffc49bc9ad4b0a98
head: f396ac7
head-fingerprint: 8f5012d6d47a6f34df899e1edd2cefbacb28ba5c
cases: 198
rounds: 2
reps: 3
date: 2026-10-29
-->

## Device comparison (CUDA): red

**Accepted:** -

`f396ac7` against `5619c3e` (origin/dev), 2026-10-29.

**VERDICT: host and device answers differ in 6 cases

<!-- END receipt cuda -->
