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
head: c248ca8
head-fingerprint: f6fccc43ac7f90a34f4d3ca466a6ced30a707094
cases: 152
rounds: 3
reps: 5
date: 2026-10-26
-->

## Host comparison: red

**Accepted:** Ray programs stored beside the Tape's clauses (about 16 bytes per expression node) grow its pool; compositions are now traced exactly instead of marched, which changes the CSG ray checksums.

`c248ca8` against `cd92804` (origin/dev), 2026-10-26.

**VERDICT: MAJOR regression in 18 cases

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: cd92804
base-fingerprint: 8796be7eaaa989420ee9fdc7626438fc4d6c35db
head: c248ca8
head-fingerprint: f6fccc43ac7f90a34f4d3ca466a6ced30a707094
cases: 152
rounds: 3
reps: 5
date: 2026-10-26
-->

## Device comparison (CUDA): red

**Accepted:** On the device, a small fused solid in float traces slower than dev marched it in registers (the span lists live in local memory); the Tape and larger scenes trace faster. A few float rays graze a surface tangentially; host and device round them differently. Checksums change because compositions are now traced exactly.

`c248ca8` against `cd92804` (origin/dev), 2026-10-26.

**VERDICT: host and device answers differ in 8 cases

<!-- END receipt cuda -->
