<!--
SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>

SPDX-License-Identifier: GPL-3.0-or-later
-->

PR #207 status: `Performance/`, probes and a per-PR comparison
=============================================================

A portable record of this pull request's work, for a reviewer (human or agent) picking it up on
another machine with only the repository and `gh`. Branch `claude/performance`, base `dev`.
Remove this file before merging.

Last updated 2026-10-21, at commit `ce15e17`.

1. What the PR is for
---------------------

Reviewing ten open PRs (#194-#203) for performance regressions against the separate
EBGeometry-benchmarks repository did not work: that suite pinned an EBGeometry 40 PRs old, needed a
port before it measured anything, and ran several structures in one process, which produced
artifacts (a hash-grid build looked 37% faster only because a smaller BVH had been built just before
it in the same process). This PR moves the measurement into the repository: `Performance/` is a third
folder of executables beside `Examples/` (teach) and `Tests/` (verify), and every PR is compared with
its base on the same machine, host and device, from one source, ending in a verdict.

The ten PRs are to be reviewed **after** this one merges, in order #194 -> #203, each rebased on the
then-current `dev` and compared against it.

2. Decisions the maintainer made (do not relitigate without asking)
------------------------------------------------------------------

| Topic | Decision |
|---|---|
| Location | Top-level `Performance/`; one program per component (probe), many named cases each. |
| Tooling | Nothing beyond CMake, git and a C++ compiler: no Python. Linux and macOS only (POSIX `isatty`, `taskset`). |
| Entry point | `Scripts/run-performance.sh` (name chosen to avoid clashing with Linux `perf`). Full suite by default, globs for subsets. Plain runs: one process per probe. Comparisons: one process per case. |
| Comparing | Each side builds its **own** `Performance/`; cases matched by name; the report names probes that differ. A baseline without `Performance/` borrows the candidate's. |
| Thresholds | minor >= 3% slower, MAJOR >= 10%, both only outside a bootstrap 95% interval. Pool bytes use the thresholds alone. |
| Verdicts | One `VERDICT:` line at the end (green / yellow / red); red exits 2 and fails CI. A minor regression is a question for the author ("investigate?"); a MAJOR one blocks merging until fixed or explicitly accepted. |
| CI | `Performance-smoke` and `Performance-compare` jobs, both required by `CI-passed`. CI catches large regressions; the local comparison (quiet machine, device) is authoritative. |
| Device | Local CUDA only for device timings (no GPU CI runner). |
| GPU builds | A host build runs `host` cases only (one pinned core, vectorised); a CUDA/HIP build runs only `hostdevice` pairs. The host side of a pair uses every hardware thread but one (OpenMP), not one core, for a fair comparison. |
| Smoke comparisons | `compare --smoke` is a correctness check (builds, every case runs, same answers), never a timing verdict; its times are shown "for information only". |
| Precision | Every case runs in both `float` and `double`. |
| Every PR | Carries its comparison report in the PR template's *Performance* section (CLAUDE.md, "Vetting every pull request for performance"). |
| Quality bar | Production quality: every file reviewed for documentation, structure and consistency (the audit, section 5). |

3. The design as it stands
--------------------------

```
Performance/
  PointCloud/main.cpp   PointCloudBVH, PointCloudHashGrid (500k points, cube and sphere)
  Mesh/main.cpp         TriMeshSDF (W=4, and host-tuned W=8), MeshSDF on the armadillo
  BVHUnion/main.cpp     ~100k spheres, diagonal capsules, random cylinders
  Common/               the harness; PerfHarness.hpp includes all of it
    Format.hpp            units, padding, terminal style, a terminal/Markdown table   (no EBGeometry)
    Statistics.hpp        quantile, median, summary, bootstrap ratio interval          (no EBGeometry)
    Schema.hpp            JSON field names (Perf::Key), writer, one-line reader        (no EBGeometry)
    Environment.hpp       commit, compiler, SIMD level, CPU, host threads
    Case.hpp              CaseName, caseName(), Case (timing loop, records, JSON)
    Inputs.hpp            xorshift generator, Perf::Seed, cube(), Morton order, Order, meshFile()
    Report.hpp            TableReport (host / GPU layouts), JsonReport
    Suite.hpp             command line, which cases a build keeps, one run loop
    Device.hpp            measureOnDevice(), measureHostAndDevice()
    Probe.hpp             SignedDistance, checksumOf(), agreeDistance/agreeHit, measureHostQuery(), measureBuild()
  Tools/PerfCompare.cpp judges two result sets (shares Format/Statistics/Schema)
  RunPerformance.cmake  the logic of Scripts/run-performance.sh
  Compare.cmake         exports two commits, builds both, interleaves runs, reports
  CMakeLists.txt        builds the probes (standalone or as a subdirectory)
Scripts/run-performance.sh   two-line wrapper: cmake -P Performance/RunPerformance.cmake -- "$@"
```

Case names: `component/input.../operation/precision/side`, side `host` or `hostdevice`. Every case
records a checksum, its code-selecting parameters (spelled out, never `Default*`) and pool bytes.
Inputs come from the harness's own generator with named seeds (identical on every platform).

Also touched: top-level `CMakeLists.txt` (`EBGEOMETRY_BUILD_PERFORMANCE`), `CMakePresets.json`
(`perf`, `perf-cuda`, test preset `perf`), `.github/workflows/CI.yml`, the PR template, `CLAUDE.md`
(sections "Adding unit tests, examples and performance cases" and "Vetting every pull request for
performance"), `Docs/Sphinx/source/{PerformanceTesting,ContinuousIntegration,index}.rst`, `conf.py`
(a LaTeX mapping for U+2501), `Performance/README.md`. No change to `Source/`.

4. Commit history
-----------------

| Commit | What |
|---|---|
| `326c283` | First version: harness, probes, Compare.cmake, PerfCompare, CI, docs |
| `5aec582` | Compare.cmake passes the device compiler on |
| `dabebcf` | Longer repetitions for two too-short cases |
| `d72fe17` | k-nearest-neighbour cases (host and device) |
| `4f1ff20` | Docs: what every case measures, where results go |
| `7a9ceae` | C++17 fix for nvcc 12.0 (captured structured bindings) |
| `5f047b4` | Readable tables, device speedup |
| `4d74917` | `Scripts/run-performance.sh` front door |
| `ee64b24` | GPU builds run pairs only; OpenMP host side |
| `c2f2cdb` | Per-item times for both sides; all hardware threads but one |
| `ff45a2c` | Probes set apart in the terminal; banner once per run |
| `f8a8176` | BVHUnion capsules and cylinders; groups in one block |
| `bc7cc52` | `compare --smoke` |
| `5655e31` | Smoke times shown for information only |
| `0185ece` | One-line VERDICT |
| `b112e3f` | Audit step 1: fixes (see section 5) |
| `17c0fd2` | Audit step 2: harness split into `Common/` |
| `5b92d71` | Audit step 3: `Probe.hpp`, probes rewritten |
| `dac9746` | Audit step 4: PerfCompare restructured, moved to `Tools/` |
| `6b29042` | clang-tidy findings (harness headers included) |
| `8bb4f09` | Docs audit fix |
| `ce15e17` | PDF docs build fixed (π, U+2501) |

5. The audit (b112e3f..6b29042) and how it was verified
-------------------------------------------------------

A static review of every C++ file found: PerfCompare could count a case as both faster and MAJOR;
a host/device disagreement showed `minor` per case but a red overall verdict (now `DIFFER`); a
leftover `--side` option; lax command-line parsing; point-cloud inputs from
`EBGeometry::Random::samplePoints`, which is not reproducible across standard libraries; heavy
duplication (helpers copied 2-9 times across probes, formatting/statistics/JSON duplicated between
harness and PerfCompare); a 1,364-line harness doing six jobs; PerfCompare deciding on strings.
Step 1 fixed behaviour (and intentionally changed the PointCloud inputs, hence their checksums,
once); steps 2-4 restructured only.

Verification of steps 2-4 against a reference recorded right after step 1 (`b112e3f`):

* host smoke run, 88 cases; CUDA smoke run, 60 pairs; full-size host run, 88 cases: identical case
  names, order, items, parameters, pool bytes, checksums and host/device mismatch counts;
* old vs new PerfCompare on real results and on results doctored to a MAJOR and a minor slowdown,
  more pool bytes on a faster case, a changed checksum, a missing case, a skipped case and a
  host/device disagreement, in full and smoke mode: the same per-case verdict and exit code
  everywhere (only the verdict line's plurals changed);
* every C++ file compiles cleanly with `-Wall -Wextra -Wpedantic -Wshadow -Wconversion
  -Wsign-conversion -Wold-style-cast -Wuseless-cast`; clang-tidy (repository config, header filter
  widened to `Performance/`) reports nothing.

To repeat the reference check: build a commit, run each probe with `--smoke --json` (and `--json
--reps 1` for full sizes), and compare the JSON lines' `case`, `items`, `params`, `pool_bytes`,
`checksum` and `metrics.mismatches` between commits (timings are expected to differ).

6. Findings and numbers (RTX 3080 Ti, CUDA 12.5; Ryzen 9 5900X, 12 cores / 24 threads)
-------------------------------------------------------------------------------------

* **#194 regressions** (found by this harness before the audit; for the #194 review):
  PointCloudBVH all-1NN +13-16% and per-point `nearestNeighbor` +22-25%, both MAJOR. Cause: in
  `PointCloudBVH::query`, a child's box is read through the child node (`getDistanceToBoundingVolume2
  (nodes[child], ...)`), an extra node load per child. Reading the parent's SIMD row directly
  (`scalarChildDistance2(row(parent.getChildBoxRow()), point, k)`) restored all-1NN; `nearestNeighbor`
  stayed about +9% (a second slow path, not yet found).
* **PointCloudHashGrid** `closestPoint` is very slow away from its points: about 9.5 ms per query
  outside the cloud's box, about 100 us inside the hollow unit sphere (BVH: under 1 us). Not filed.
* **Host vs device**: in float the device is 4-23x faster than 23 host threads; in double 0.9-3.5x
  (this card's FP64 runs at 1/64 of FP32). In a CUDA build the host side is scalar (nvcc cannot
  compile host SIMD with GCC 12/13 headers); against a vectorised host, the float lead would be
  roughly 4-7x (SIMD is worth ~3.4x per core for TriMeshSDF). Every host/device pair agrees.
* **Threads**: 23 of 24 hardware threads was fastest everywhere; 24 lost whole scheduler ticks
  (times landing on whole milliseconds), 12 gave away the hyperthreading gain (22-27% on heavy
  queries).
* **Noise**: with 60-88 cases at a 95% interval, an occasional false minor (or faster) is expected;
  rerun a flagged case with `--rounds 10` before calling it a regression. Smoke times vary by up to
  ±40% on identical code, which is why smoke comparisons judge no timings.
* **Durations**: full host comparison about 10 min; CUDA comparison about 5 min; smoke comparison
  30-70 s.

7. Status and open work
-----------------------

* **CI**: no run had passed before `ce15e17`; every completed run failed in `Build-documentation`
  (the PDF build stopped on 'π' and U+2501). On `ce15e17`, `Build-documentation` and
  `Performance-smoke` and 20 other jobs pass; GPU HIP compile, Performance-compare, the Release build, the
  sanitizers and clang-tidy were still running when this file was written. The HIP compile
  of the restructured probes (audit steps 2-4) has not completed on any commit yet; it cannot be
  built locally. Check: `gh pr checks 207`.
* **Full host comparison** (88 cases) since the audit: not run. Run it into a separate work
  directory (below) and replace the stale note in the PR's *Performance* section.
* **PR description**: rewritten at `8bb4f09` to match the current design; its *Performance* section
  carries the CUDA comparison measured at `f8a8176` (before the audit; still representative).
* The PR is marked ready for review although CI is not yet green.
* Afterwards: review #194-#203 in order (section 1). #203 sits on #201; the others on `dev`. Each
  needs rebasing onto `dev` once this merges, and #194 needs a probe update (it changes PackedBVH
  traversal APIs the probes do not call, so likely none) before its comparison.
* Related issues: #206 (faster sphere tracing of CSG tapes; recorded for later), #208 (nightly
  performance tracking with this harness).

8. How to run things
--------------------

```bash
Scripts/run-performance.sh --help
Scripts/run-performance.sh                                  # every host case
Scripts/run-performance.sh 'PointCloudBVH/*' --reps 10      # a subset
Scripts/run-performance.sh compare origin/dev --smoke        # one-minute correctness check
Scripts/run-performance.sh compare origin/dev                # full host comparison (~10 min)
Scripts/run-performance.sh compare origin/dev --gpu cuda --gpu-arch 86 --cuda-compiler <nvcc>
ctest --preset perf                                         # what CI's smoke job runs
```

A comparison through the script always writes to `build/performance/compare/` and replaces what was
there. To keep someone else's results, run the comparison directly with its own work directory:

```bash
cmake -DBASE=origin/dev -DHEAD=WORKTREE -DWORK_DIR=/some/other/dir -P Performance/Compare.cmake
```

(variables are listed at the top of `Performance/Compare.cmake`; `--out` for the script is a
possible improvement).

9. Pitfalls met along the way
-----------------------------

* `build/performance/compare/` may hold the maintainer's own results: look at its `report.md` first,
  in a separate step, and use another work directory for your own checks (it was overwritten twice).
* Build both Sphinx outputs after touching `.rst` files: HTML (`pre-commit run sphinx-build-html
  --hook-stage manual --all-files`) **and** PDF (`sphinx-build-pdf`). pdfLaTeX rejects characters
  the HTML build accepts.
* A stray `sphinx-autobuild` rebuilding into the same directory makes Sphinx builds fail
  intermittently (`pgrep -fa sphinx`).
* An untracked file in the repository root without SPDX headers fails the REUSE hook for every
  commit; the last commits were made with `SKIP=reuse` because of a local, untracked `shot_root.png`.
* CUDA 12.0's `nvcc` (the distribution's `/usr/bin/nvcc` on the original machine) cannot compile the
  library; use CUDA 12.5 or newer (on the original machine:
  `/opt/nvidia/hpc_sdk/Linux_x86_64/24.7/compilers/bin/nvcc`, architecture 86).
* In CMake script mode, `if(n)` reads `n` as the false constant N: never name a variable n, y, on, off.
* nvcc rejects lambdas capturing structured bindings in C++17 (GCC and newer nvcc accept them).
