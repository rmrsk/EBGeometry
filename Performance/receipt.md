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
base: 9d36e55
base-fingerprint: df44594dbe8bca38016afd40f600a0df5cd4f090
head: 46ec20c
head-fingerprint: 8796be7eaaa989420ee9fdc7626438fc4d6c35db
cases: 144
rounds: 3
reps: 5
date: 2026-10-25
-->

## Host comparison: yellow

**Accepted:** Watertightness fix: rays no longer slip between faces sharing an edge; the widening costs the triangle test a few percent, 20% in FP64 on the device.

`46ec20c` against `9d36e55` (origin/dev), 2026-10-25.

**VERDICT: 6 minor regressions

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: 9d36e55
base-fingerprint: df44594dbe8bca38016afd40f600a0df5cd4f090
head: 46ec20c
head-fingerprint: 8796be7eaaa989420ee9fdc7626438fc4d6c35db
cases: 136
rounds: 3
reps: 5
date: 2026-10-25
-->

## Device comparison (CUDA): red

**Accepted:** Watertightness fix: rays no longer slip between faces sharing an edge; the widening costs the triangle test a few percent, 20% in FP64 on the device. 2 rays graze a drill rim within the march tolerance; host and device round differently. Expected for marched rays.

`46ec20c` against `9d36e55` (origin/dev), 2026-10-25.

**VERDICT: host and device answers differ in 4 cases

<!-- END receipt cuda -->
