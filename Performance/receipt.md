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
base: ca218bb
base-fingerprint: ee5de35b8fe17a5503f71bbbcab6df079f6783f4
head: 0173868
head-fingerprint: 15204015c7463e87f984661ed87db6a6feaa9c85
cases: 108
rounds: 3
reps: 5
date: 2026-10-23
-->

## Host comparison: yellow

**Accepted:** Generic Tape deficiencies, as already noted in issues #210, #224 and #221. At some point, the Tape will be set up as a compile-time geometry, as outlined in issue #224.

`0173868` against `ca218bb` (origin/dev), 2026-10-23.

**VERDICT: 2 minor regressions

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: ca218bb
base-fingerprint: ee5de35b8fe17a5503f71bbbcab6df079f6783f4
head: 0173868
head-fingerprint: 15204015c7463e87f984661ed87db6a6feaa9c85
cases: 80
rounds: 3
reps: 5
date: 2026-10-23
-->

## Device comparison (CUDA): red

**Accepted:** Generic Tape deficiencies, as already noted in issues #210, #224 and #221. At some point, the Tape will be set up as a compile-time geometry, as outlined in issue #224.

`0173868` against `ca218bb` (origin/dev), 2026-10-23.

**VERDICT: MAJOR regression in 1 case

<!-- END receipt cuda -->
