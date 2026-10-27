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
base: e0a6775
base-fingerprint: 861f41a57d75a617c77888fbeccb32f342198aa4
head: 2fe5b8c
head-fingerprint: 7cbe8dbd0203e980bb20b3345bd0f1e8985d5f47
cases: 134
rounds: 2
reps: 3
date: 2026-10-27
-->

## Host comparison: yellow

**Accepted:** Deemed immaterial.

`2fe5b8c` against `e0a6775` (origin/dev), 2026-10-27.

**VERDICT: 2 minor regressions

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: e0a6775
base-fingerprint: 861f41a57d75a617c77888fbeccb32f342198aa4
head: 2fe5b8c
head-fingerprint: 7cbe8dbd0203e980bb20b3345bd0f1e8985d5f47
cases: 160
rounds: 2
reps: 3
date: 2026-10-27
-->

## Device comparison (CUDA): red

**Accepted:** Deemed immaterial. A few float rays graze a surface tangentially; host and device round them differently (4 of 262,144 Render pixels, as before this PR's ray changes).

`2fe5b8c` against `e0a6775` (origin/dev), 2026-10-27.

**VERDICT: host and device answers differ in 7 cases

<!-- END receipt cuda -->
