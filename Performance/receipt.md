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
base: cd92804
base-fingerprint: 8796be7eaaa989420ee9fdc7626438fc4d6c35db
head: a87aa41
head-fingerprint: 73c6eb0348bdf2bf808bf641ba3840cfc1cd010e
cases: 120
rounds: 2
reps: 3
date: 2026-10-26
-->

## Host comparison: red

**Accepted:** No library change; the flagged cases are machine noise (a second session was running): rechecked with 10 rounds, both within noise.

`a87aa41` against `cd92804` (origin/dev), 2026-10-26.

**VERDICT: MAJOR regression in 1 case

<!-- END receipt host -->
