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
base: 5e3febe
base-fingerprint: 866e6a5180c53f362ab3ae9e222322a43f91e901
head: 97348da
head-fingerprint: 16463cbe993d1610f833c1c5ff05e8aaa19ed961
cases: 88
rounds: 3
reps: 5
date: 2026-10-22
-->

## Host comparison: red

**Accepted:** This PR changes how builds are timed (glibc heap only, freed memory returned before each repetition), so its base and head sides measure builds differently; the build changes are the new measurement, not the library (queries do not move). The library is identical on both sides, and the extra build time is the extra page faults of a first build, at about 0.9 us each. Accepted by the maintainer.

`97348da` against `5e3febe` (origin/dev), 2026-10-22.

**VERDICT: MAJOR regression in 9 cases

<!-- END receipt host -->
