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
base: e3253fc
base-fingerprint: 704951f9511e922b0cc01e21a2a8cd31b91101d2
head: 3ee0503
head-fingerprint: 48c61b42ee97f3c8b09fc1ceffc49bc9ad4b0a98
cases: 136
rounds: 2
reps: 3
date: 2026-10-29
-->

## Host comparison: yellow

**Accepted:** #246 changes no host code. The four Tape minors (smooth float and double, chain16 float, csg double, +3% to +6%) are code placement: with `-falign-functions=64` the base and the PR take the same time. The Ray/csg-tape/firstHit/float minor (+3.1%) did not hold over 10 rounds.

`3ee0503` against `e3253fc` (origin/dev), 2026-10-29.

**VERDICT: 5 minor regressions

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: e3253fc
base-fingerprint: 704951f9511e922b0cc01e21a2a8cd31b91101d2
head: 3ee0503
head-fingerprint: 48c61b42ee97f3c8b09fc1ceffc49bc9ad4b0a98
cases: 186
rounds: 2
reps: 3
date: 2026-10-29
-->

## Device comparison (CUDA): red

**Accepted:** The two MAJORs, RayCSG/scene-fused/firstHit-morton/float (+21%) and Render/mesh/frame/float (+10%), are the cost of the default minimum of 384 resident threads. It lets the compiler give these kernels up to 168 registers, and it uses 161, up from 56 and 128 on the base, so fewer of their threads fit. That is the trade for fixing the register budget, which speeds up 50 cases. A caller can pick another minimum for such a kernel (Examples/DeviceTuning). The 6 host/device disagreements are one ray each, and the base gives the same answers, so they predate this PR.

`3ee0503` against `e3253fc` (origin/dev), 2026-10-29.

**VERDICT: host and device answers differ in 6 cases

<!-- END receipt cuda -->
