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
base: 58a78db
base-fingerprint: e82f696417f53100ce7598a10c5f88662693de2f
head: 57aea1e
head-fingerprint: b994d5a15d03f7c986c18ff5975e90555d814775
cases: 124
rounds: 3
reps: 5
date: 2026-10-24
-->

## Host comparison: yellow

**Accepted:** false regression

`57aea1e` against `58a78db` (origin/dev), 2026-10-24.

**VERDICT: 1 minor regression

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: 58a78db
base-fingerprint: e82f696417f53100ce7598a10c5f88662693de2f
head: 57aea1e
head-fingerprint: b994d5a15d03f7c986c18ff5975e90555d814775
cases: 96
rounds: 3
reps: 5
date: 2026-10-24
-->

## Device comparison (CUDA): yellow

**Accepted:** noise; 10-round rerun -1.4%

`57aea1e` against `58a78db` (origin/dev), 2026-10-24.

**VERDICT: 1 minor regression

<!-- END receipt cuda -->
