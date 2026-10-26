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
base: 8f43bf5
base-fingerprint: 73c6eb0348bdf2bf808bf641ba3840cfc1cd010e
head: beea848
head-fingerprint: 861f41a57d75a617c77888fbeccb32f342198aa4
cases: 130
rounds: 2
reps: 3
date: 2026-10-26
-->

## Host comparison: red

**Accepted:** Ray programs stored beside the Tape's clauses (about 16 bytes per expression node, and 8 per sub-program) grow its pool; compositions are now traced exactly instead of marched, which changes the CSG ray checksums. closestPrimitive() on a tape whose primitives sit past the host dispatch split (fused parts, meshes, user types) is about 6% slower; a follow-up tunes the split.

`beea848` against `8f43bf5` (origin/dev), 2026-10-26.

**VERDICT: MAJOR regression in 18 cases

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: 8f43bf5
base-fingerprint: 73c6eb0348bdf2bf808bf641ba3840cfc1cd010e
head: beea848
head-fingerprint: 861f41a57d75a617c77888fbeccb32f342198aa4
cases: 156
rounds: 2
reps: 3
date: 2026-10-26
-->

## Device comparison (CUDA): red

**Accepted:** On the device, a small fused solid in float traces slower than dev marched it in registers (the span lists live in local memory); the Tape and larger scenes trace faster. A few float rays graze a surface tangentially; host and device round them differently. Checksums change because compositions are now traced exactly.

`beea848` against `8f43bf5` (origin/dev), 2026-10-26.

**VERDICT: host and device answers differ in 6 cases

<!-- END receipt cuda -->
