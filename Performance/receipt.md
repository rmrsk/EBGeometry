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
base: 8e03593
base-fingerprint: 15204015c7463e87f984661ed87db6a6feaa9c85
head: 9f9f57a
head-fingerprint: 00a3c8eb2918da445723bc4331d4474b6739daa2
cases: 118
rounds: 3
reps: 5
date: 2026-10-24
-->

## Host comparison: yellow

**Accepted:** -

`9f9f57a` against `8e03593` (origin/dev), 2026-10-24.

**VERDICT: 3 minor regressions

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: 8e03593
base-fingerprint: 15204015c7463e87f984661ed87db6a6feaa9c85
head: 9f9f57a
head-fingerprint: 00a3c8eb2918da445723bc4331d4474b6739daa2
cases: 90
rounds: 3
reps: 5
date: 2026-10-24
-->

## Device comparison (CUDA): yellow

**Accepted:** -

`9f9f57a` against `8e03593` (origin/dev), 2026-10-24.

**VERDICT: 3 minor regressions

<!-- END receipt cuda -->
