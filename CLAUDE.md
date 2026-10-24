# CLAUDE.md

Guidance for Claude Code (or any agent) working in this repository.

## What this is

EBGeometry is a **header-only C++17 library** for:

1. Turning watertight, orientable surface meshes into signed distance functions (SDFs), via a
   half-edge (DCEL) mesh or raw triangle representation.
2. Fast SDF evaluation using bounding volume hierarchies (BVHs) — both a pointer-based `TreeBVH`
   and a flattened, SIMD-friendly `PackedBVH`.
3. BVH-accelerated constructive solid geometry (CSG) unions of many objects.
4. Analytic SDFs/implicit functions (spheres, boxes, etc.) and transforms (union, intersection,
   rounding, blending, rotation/translation/scaling).

It has zero external dependencies at use-time: consumers just need `EBGeometry.hpp` (which
`#include`s everything under `Source/`) on their include path and a C++17 compiler. It was written
for embedded-boundary (EB) codes like AMReX, but is general-purpose.

Single-word orientation: `Source/` is the library (all headers, no `.cpp`), `Tests/` is the Catch2
unit-test suite (fetched via CMake `FetchContent`, not vendored), `Examples/` holds small
standalone programs with no third-party dependencies (each independently buildable — see below),
`Performance/` holds the performance probes and the comparison that every PR is vetted with (see
"Vetting every pull request for performance" below), `Integrations/` holds illustrative (not
CI-tested) examples coupling EBGeometry to third-party application codes (AMReX, Chombo), and
`Docs/Sphinx/` + `Docs/doxygen.conf` are the two documentation systems.

## Cloning

```bash
git clone --recurse-submodules git@github.com:rmrsk/EBGeometry.git
```

The `common-3d-test-models` submodule provides mesh files (`armadillo.obj`, `cow.obj`, ...) used
by the `MeshSDF` example and the mesh integrations. If cloned without
`--recurse-submodules`, run `git submodule update --init` afterward. It is **not** needed to build
or test the library itself — only to run those with their default (no-argument) input.

## Compiling

Three independent, equally-supported ways to build any `Examples/<something>` program
(see `Examples/README.md` and each example's own `README.md`); all three now place the resulting
binary in the same location: `<Example>.ex`, directly in that example's own folder.

```bash
# 1. Direct compiler invocation
cd Examples/Shapes
g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o Shapes.ex

# 2. GNU make
cd Examples/Shapes
make                                    # override with PRECISION=float, EBGEOMETRY_HOME=...

# 3. CMake (standalone, this example only)
cd Examples/Shapes
cmake -S . -B build && cmake --build build
```

Every example except `BuildBVH` and `HostTuning` also takes a GPU switch (`make GPU=HIP|CUDA`, CMake
`-DEBGEOMETRY_EXAMPLE_GPU=HIP|CUDA`, or the project's `EBGEOMETRY_ENABLE_HIP`/`CUDA`): the same source
then runs its last section's query functor on the device through `GPU::transform()` (in a plain C++
build, the same section runs as a host loop). A GPU build with no visible GPU skips the device part
and exits 0. Examples check nothing themselves; device correctness is the `[gpu]` unit tests' job. In a GPU build of the
project these examples are collected in `EBGeometry_GPUDeviceExamples` (CI compiles it, without
running it); their tests keep the `example` label.

All three accept an `EBGEOMETRY_PRECISION`/`PRECISION` override (`float` or `double`, default
`double`), an `EBGEOMETRY_HOME` override (default `../..`, the path to the directory containing
`EBGeometry.hpp`), and an `EBGEOMETRY_ENABLE_ASSERTIONS`/`ASSERTIONS` toggle (`ON`/`OFF`, default
`OFF`) for `EBGEOMETRY_EXPECT()` runtime assertions, so an example folder can be copied out as a
standalone project template.

### Building the whole project (library target + all tests + all examples)

Use the CMake presets (CMake ≥ 3.22 required; each preset gets its own isolated
`build/<preset-name>/` directory):

```bash
cmake --preset debug            # Debug, assertions ON, no SIMD -- use this for development
cmake --build --preset debug --parallel $(nproc)

ctest --preset debug            # unit tests only, ~15 s
ctest --preset examples         # run every example via ctest, several minutes in Debug mode
```

| Preset          | Build type | Assertions | SIMD | Tests/examples built |
|-----------------|------------|------------|------|-----------------------|
| `debug`         | Debug      | ON         | none | ON |
| `debug-san`     | Debug      | ON         | none | ON, + ASan/UBSan |
| `release`       | Release    | OFF        | avx  | OFF |
| `release-test`  | Release    | OFF        | avx  | ON |
| `perf`          | Release    | OFF        | avx + `-march=native` | `Performance/` probes only |

`EBGEOMETRY_SIMD` (`none|sse41|avx|avx512`) and `EBGEOMETRY_ENABLE_ASSERTIONS` are ordinary CMake
cache variables if you need a combination not covered by a preset.

## Testing

```bash
ctest --preset debug                 # ~430 unit tests (Catch2), ~15 s
ctest --preset debug-san             # same, under AddressSanitizer + UBSan
ctest --preset examples              # every Examples/* program, run to completion
ctest --preset release-test          # unit tests + examples, optimised build
```

Test sources live in `Tests/Test*.cpp` (one file per component: `TestVec`, `TestBoundingVolumes`,
`TestDCEL`, `TestBVH`, `TestPolygonSoup`, `TestParser`, ...); fixture files are in
`Tests/data/`. `Tests/InstantiateAll.cpp` is a compile-only target (never run) that explicitly
instantiates every public class template for both `float` and `double` — it exists purely so
clang-tidy and `-Wdouble-promotion`/friends analyse both precisions; add new public classes there
when you add them to the library.

Most test files are written with `TEMPLATE_TEST_CASE(..., EBGEOMETRY_TEST_PRECISIONS)` (see
`Tests/TestFloatingPointUtils.hpp`) rather than a hardcoded `double`, so they can run under both
precisions -- but locally, by default, only `double` actually runs (fast iteration, matching
whatever the CMake preset otherwise builds). CI configures with
`-DEBGEOMETRY_TEST_BOTH_PRECISIONS=ON` (`Tests/CMakeLists.txt`'s
`EBGEOMETRY_TEST_BOTH_PRECISIONS` option) to additionally verify `float` *behaviorally* on every
push, not just that it compiles via `InstantiateAll.cpp`. When adding a new test, prefer
`TEMPLATE_TEST_CASE` and `EBGEOMETRY_TEST_PRECISIONS` over a hardcoded `double`; use
`tightMargin<T>()`/`looseMargin<T>()`/`withinAbsT<T>(...)` from `TestFloatingPointUtils.hpp` for
`WithinAbs` comparisons, since (unlike `WithinRel`) it has no type-specific default margin.

To run a single test file or a specific Catch2 test case, build the individual binary and invoke
it directly (each test binary supports the full Catch2 CLI, e.g. `-l` to list, name filters, `-s`
for successful-assertion output):

```bash
cmake --build --preset debug --target TestBVH
./build/debug/Tests/TestBVH
```

## Documentation

Two independent systems; both are also pre-commit hooks (see below) and CI jobs.

```bash
# Doxygen API reference (warnings treated as errors)
cd Docs && doxygen doxygen.conf

# Sphinx user guide -- HTML and PDF
cd Docs/Sphinx
make html      # output: Docs/Sphinx/build/html/index.html
make latexpdf   # requires a LaTeX toolchain
```

Sphinx is pinned to `sphinx==5.3.0` (see `.pre-commit-config.yaml` and
`.github/workflows/{CI,docs}.yml`) — not to a specific `sphinxcontrib-bibtex` requirement (bibtex
only requires `Sphinx>=3.5`, no upper bound), but because **`sphinx==5.0.0` has a real internal
crash bug**: its generic cross-reference-resolution post-transform (`ReferencesResolver`) throws
`AttributeError: 'Text' object has no attribute 'rawsource'` while deep-copying the content node
of a `:ref:`/`:any:` reference that has no explicit link text — a totally standard, correct usage
pattern used throughout this doc set, not a docs bug. `5.3.0` (the last 5.x release) fixes it with
no other observable behavior change; installing/using a different Sphinx version standalone can
still produce stale or incompatible `Docs/Sphinx/build/` doctree caches, so stick to this pin
rather than a system-installed Sphinx. **If a Sphinx build throws an internal error in
`sphinx.environment.collectors.toctree`, or `rm -rf Docs/Sphinx/build` fails with "Directory not
empty," check for a stray `sphinx-autobuild` (or other long-running `sphinx-build`) process first
(`pgrep -fa sphinx`) — a forgotten one rebuilding into the same output directory in the background
has caused exactly this before.**

## When you change code, check whether the docs still agree with it

Sphinx docs and Doxygen comments describe the code but aren't checked by the compiler, so they
silently drift out of sync with it. This repository used to embed source excerpts directly into
the Sphinx docs via `.. literalinclude::`, and a line-number shift in the source (e.g. adding a
field to a constructor) could silently make a `:lines:` range cite the wrong span -- nothing
failed to compile, no test failed, and the only reason it was ever caught was a manual full Sphinx
rebuild and a careful read of the rendered excerpt.

**`literalinclude` is banned in `Docs/Sphinx/source/*.rst`.** Every occurrence has been removed;
never add a new one. Where a reader needs the exact C++ signature of a class or function, link to
its Doxygen-generated page instead (e.g.
`` `TreeBVH <doxygen/html/classEBGeometry_1_1BVH_1_1TreeBVH.html>`__ ``) rather than showing source
inline -- Doxygen is regenerated from the header itself on every build, so it cannot drift the way
a hand-maintained line range could. Before adding a Doxygen link, verify the target actually
exists: build Doxygen locally (`cd Docs && doxygen doxygen.conf`) and check the generated `.html`
filename/anchor under `Docs/doxygen/html/`, since a wrong mangled class name or a stale
member-function anchor produces a silently-broken link.

**The Sphinx docs must be updated before a PR is merged, not left for later.** If a change alters
a documented class/function's behavior, signature, or existence, the corresponding
`Docs/Sphinx/source/*.rst` page(s) must be updated in the same PR -- an undocumented behavior
change is not a finished change.

**Never point users to tests (`Tests/*`) to see a functionality demonstrated.** The unit tests
exist to verify behavior, not to teach it. When the docs (`.rst` files) or a public header comment
want to show a user how to *use* something, they must point to an `Examples/` program instead --
and only to one that actually exists as a page in the Sphinx Examples section (the toctree in
`Docs/Sphinx/source/index.rst`, e.g. `ExampleShapes.rst`). If no such example exists yet, either
add one (with its `Example*.rst` page wired into that toctree) or describe the capability inline
without citing any file -- do not fall back to citing a test. (Pointing to `Tests/` is fine only
in genuinely contribution/testing-oriented docs like `ContributionGuidelines.rst`/
`TestingLocally.rst`, which are about the tests themselves, not about demonstrating a feature.)

**Claude must begin auditing the `.rst` docs as soon as a PR is marked ready for review (i.e. no
longer a draft), not only when explicitly asked.** Check the PR's draft status (e.g.
`gh pr view <number> --json isDraft`); once it is out of draft, treat a documentation audit of
that PR's changes as part of finishing the work, following the steps below, even if the user
hasn't separately requested a docs pass.

Before treating a code change as finished, look at what you actually staged/changed
(`git diff --staged --name-only`, or `git diff HEAD --name-only` if not yet staged) and, for each
changed `Source/*.hpp` file:

1. **Confirm no `literalinclude` crept back in**: `grep -rn "literalinclude" Docs/Sphinx/source/`
   must return nothing. If it does, replace it with a Doxygen link as described above.
2. **Grep for renamed/removed identifiers** in prose: if you renamed a class, method, or
   parameter, `grep -rn "<oldName>" Docs/Sphinx/source/ Docs/mainpage.md README.md CLAUDE.md` to
   catch stale references -- a Doxygen link doesn't automatically break when the linked class is
   renamed the way a literalinclude would, so this needs an explicit check.
3. **If you added a new public class/function**, add it to `Tests/InstantiateAll.cpp` (for
   clang-tidy/compile coverage under both precisions, per the existing convention) and check
   whether it deserves a mention on the relevant `Docs/Sphinx/source/Implem*.rst` implementation
   page (concrete API, Doxygen-linked) and/or its `Docs/Sphinx/source/*.rst` Concepts-section
   counterpart (conceptual picture, no implementation classes named) and/or a row in a
   `Docs/Sphinx/source/TestingLocally.rst`-style coverage table, matching how existing sibling classes are
   documented.
4. **If you changed a function's signature** (added/removed/renamed a parameter, changed a return
   type), update its Doxygen `@param`/`@tparam`/`@return` comment in the same header -- `doxygen
   doxygen.conf` (`WARN_AS_ERROR = FAIL_ON_WARNINGS`) will catch a genuinely missing `@param` for a
   new parameter, but won't catch a stale description that no longer matches what the parameter
   does, or a `Docs/Sphinx/source/*.rst` page that still describes the old signature.
5. **Rebuild the Sphinx HTML docs** (see below) and check for build warnings -- a broken internal
   cross-reference (`:ref:`/`:numref:`/`:eq:`) will not stop a plain `make html`, but usually
   surfaces as a warning, and CI's Sphinx build (like the `sphinx-build-html` hook) runs with `-W`,
   so any warning fails it.

The `check-docs` pre-commit hook (`Scripts/CheckDocs.py`, default stage: it runs on every commit
that changes a `Docs/Sphinx/source/*.rst` file, and CI's Doxygen-check job runs it too) enforces the
ban mechanically: it fails if any `.. literalinclude::` directive exists anywhere under
`Docs/Sphinx/source/`. Treat it as a floor, not a replacement for the steps above -- it only
catches the banned directive itself, never "this page no longer describes what the code does,"
which needs the manual read-through above.

## Reproducing CI locally / before pushing

```bash
Scripts/run-all-checks.sh
```

Runs every pre-commit hook (default and manual stage — formatting, REUSE/license headers,
codespell, Doxygen, the `check-docs` literalinclude ban, the `check-device-math` device toolchain
contract, clang-tidy, a debug-preset compile check, the documentation figures, Sphinx HTML/PDF)
followed by all four CMake presets' test suites. This is nearly the check set
`.github/workflows/CI.yml` runs, just local and in one command; CI's `CMake-consumer` job is not
included (run `Scripts/check-cmake-consumer.sh` for that). Expect it to take several
minutes. `pre-commit install` is only needed if you want the default-stage hooks
(formatting, license, codespell, Doxygen, check-docs, check-device-math) to run automatically on `git commit`; the
manual-stage hooks (clang-tidy, the debug build, the documentation figures, Sphinx) only run via
this script or explicit `pre-commit run --hook-stage manual`. The Doxygen hook runs whenever a
header, `EBGeometry.hpp`, `Docs/mainpage.md` or `Docs/doxygen.conf` changes; codespell covers
`Source/`, `Docs/`, `Examples/` (except `Examples/data/`), `Tests/` (except `Tests/data/`), `Integrations/`, `Scripts/`,
`.github/` and the top-level text files.

`.clang-tidy` and `.pre-commit-config.yaml` define the static-analysis/formatting rules; CI
(`.github/workflows/CI.yml`) runs the same hooks plus a much larger build/test matrix (multiple
compilers, SIMD levels, precisions, sanitizers, both Debug and Release) that isn't practical to
reproduce byte-for-byte locally. CI runs on pull requests and on pushes to `main` and `dev`. Its
`CI-passed` job, the one to require for merging, fails whenever any job it depends on fails, is
cancelled or is skipped; `GPU-HIP` (a device compile) is one of those required jobs, while
`GPU-CUDA` and `Static-analysis` are advisory (`continue-on-error`).

## Whitespace conventions clang-format does not enforce

clang-format handles indentation, brace placement, and line wrapping, but it cannot insert blank
lines to group statements, so a few readability conventions are maintained by hand:

- **A blank line both before and after every loop** (`for`/`while`), not just after it, so the loop
  reads as its own block separated from the surrounding statements. Do not stack the blank line
  directly against an enclosing closing brace (a loop that is the last statement in its block needs
  no blank line between it and that closing brace).

## Adding unit tests, examples and performance cases

Three folders hold executables, each with one job: `Tests/` **verifies** (it fails when the library is
wrong), `Examples/` **teaches** (it shows a user the calls to write and checks nothing), and
`Performance/` **measures** (it times and compares, and checks only that answers did not change). Put
a new program in the folder whose job it does; never make one do another's job (an example that
asserts, a test that times, a probe that teaches).

**What a change needs.** A new public class or function: unit tests, an entry in
`Tests/InstantiateAll.cpp`, and Sphinx docs (see the docs section above). If it is a structure with a
build or a query users call in bulk: performance cases. If it is something users need shown how to
use (a new capability, not a new overload): an example. A new device path for existing code: a
`[gpu]` test and a `hostdevice` performance pair. A bug fix: a test that fails without the fix.

### Unit tests (`Tests/`)
- One file per component, `Tests/Test<Component>.cpp`, registered with `ebgeometry_add_test(...)` in
  `Tests/CMakeLists.txt`. Add to the existing file when the component exists; a new file only for a
  new component, with a row in the coverage table of `TestingLocally.rst`.
- `TEMPLATE_TEST_CASE("<Class>: <behaviour in words>", "[<Class>]", EBGEOMETRY_TEST_PRECISIONS)`, not a
  hardcoded `double`; `WithinAbs` through `tightMargin<T>()`/`looseMargin<T>()`/`withinAbsT<T>()`
  (`TestFloatingPointUtils.hpp`). Name the behaviour, not the method ("rejects an empty cloud", not
  "test constructor").
- Check against an independent reference (brute force, a closed form, a hand-computed value), never
  against the code under test run a second way.
- A device-callable function gets a `[gpu]`-tagged case that runs it through `GPU::transform`
  (`TestGPU.hpp`) and compares with the host; add the file to `EBGEOMETRY_GPU_TESTS` if it is new
  there. It runs in emulation on host builds and as a real kernel in GPU builds.
- An `EBGEOMETRY_REQUIRE` gets a death test with `abortsWith()` (`TestDeath.hpp`), matching the message.
- Inputs: small files in `Tests/data/` (never the `common-3d-test-models` submodule, which tests must
  not need), or meshes built in code (`TestConstructions.hpp`). Shared helpers go in a `Tests/*.hpp`.
- Fast: the whole `ctest --preset debug` stays around 15 s in Debug. A slow case is a probe, not a test.

### Examples (`Examples/`)
- Only for something a user needs shown. One folder per topic: `main.cpp`, `CMakeLists.txt`,
  `GNUmakefile`, `README.md`, copied from the nearest existing example so all three build routes,
  `PRECISION`, `ASSERTIONS`, `EBGEOMETRY_HOME` and the GPU switch keep working. The folder is picked up
  by CMake and CI automatically.
- It also needs a page, `Docs/Sphinx/source/Example<Name>.rst`, in the Examples toctree of `index.rst`,
  and a line in `Examples/README.md`'s reading order. Docs and headers cite examples, never tests.
- Written to be read: the calls in the order a user writes them, with comments saying why, and printed
  results a reader can follow. It checks nothing (that is the tests' job) and exits 0.
- A device-capable example has the four parts in order (the query functor, the construction, host use,
  the same query through `GPU::transform`) and skips the device part without a GPU.
- No third-party dependencies (they go in `Integrations/`); the default input is `Examples/data/` or
  the submodule; it runs in seconds in a Release build and within the 300 s ctest limit in Debug.

### Performance cases (`Performance/`)
The coverage rules are in "Vetting every pull request for performance" below. Mechanically:
- New cases go in the component's probe (`Performance/<Component>/main.cpp`); a new component gets a
  new folder with a `main.cpp` (picked up automatically) and a row in `Performance/README.md` and
  `PerformanceTesting.rst`.
- Each case: `suite.add("<component>/<input>/<operation>/<precision>/<host|device|hostdevice>", ...)`;
  set up untimed, time it with the helpers in `Performance/Common/Probe.hpp` (`measureHostQuery()`,
  `measureBuild()`) or `Case::measure()`, and record `checksum()` (`checksumOf()`), `param()` for every
  code-selecting parameter, and `poolBytes()`. A query that runs on a device gets, beside its `host`
  case, a `hostdevice` pair: a trivially copyable functor timed on both sides with
  `measureHostAndDevice()` (`Common/Device.hpp`) over `Perf::Orders`, judged by `agreeDistance()`,
  `agreeHit()` or the probe's own rule. A host build keeps only `host` cases and a GPU build only pairs
  (`Suite::add` decides). Sizes and code-selecting parameters are named constants at the top of the
  probe, random inputs use `Perf::pointsInBox()` with a seed from `Perf::Seed` (never the standard
  library's distributions), and every case has a `--smoke` size.
- Run `ctest --preset perf` and `Scripts/run-performance.sh compare origin/<base>` before committing;
  the new cases show as *new*.

## Vetting every pull request for performance

**Claude runs a performance comparison on every pull request as soon as it is opened (draft or not),
and again after any push that changes the library or the probes, without being asked.** `Performance/` holds the
probes (one program per component, many named cases, `component/input/operation/precision/side`);
`Scripts/run-performance.sh` is the one way in (its logic is `Performance/RunPerformance.cmake` and
`Performance/Compare.cmake`, needing only CMake, git and a compiler; `--help` lists everything); the
full description is `Docs/Sphinx/source/PerformanceTesting.rst`.

1. **Run the comparison against the PR's base**, from the repository root (`--smoke` first gives a
   one-minute check that both sides build and every case runs with the same answers):

   ```bash
   Scripts/run-performance.sh compare origin/<base> --record                       # host cases, receipt
   Scripts/run-performance.sh compare origin/<base> <pr-branch> --gpu cuda --gpu-arch 86 \
                              --cuda-compiler <nvcc>                               # + device
   ```

   Add the device run whenever the PR touches code a device runs (`EBGEOMETRY_HOST_DEVICE`
   functions, `GPU::`, the pool/`rebasedView` machinery, the packed layouts). It compares the
   `hostdevice` pairs, which also report the host's time on every hardware thread but one, the device's speedup and
   whether the host and device answers agree. Nothing else should be running on the machine; each
   case runs in a fresh process, alternating between the sides (a host comparison pinned to one core). (`Scripts/run-performance.sh` with no command
   runs every case of the current tree, and a glob such as `'PointCloudBVH/*'` runs a subset.)
   Run the host comparison on the PR branch with its code committed and `--record`: it writes the
   PR's **performance receipt**, `Performance/receipt.md` (its host section; the device run with
   `--record` adds a device section), which CI requires (see 4). It refuses a smoke run, a glob,
   uncommitted changes to the library or the probes, or a run with skipped cases (a fresh worktree needs
   `git submodule update --init` for the mesh cases).
2. **Report the result to the user in the terminal** (the table and the "Run it manually" block that
   the comparison prints), **commit the receipt** to the PR branch (pushing it, like any push, needs
   the user's go-ahead), **and carry the figures in the PR**: paste `build/performance/compare/report.md`
   into the *Performance* section of the PR body (`.github/pull_request_template.md`), replacing an
   older report. A PR that changes neither the library nor the probes keeps the receipt it inherited.
3. **Act on the verdicts** (the comparison ends with one `VERDICT:` line, green, yellow or red; a change
   counts only outside the 95% bootstrap interval):
   - **minor** (3% to 10% slower, or 3% to 10% more pool bytes): tell the user which cases, by how
     much, and the likely cause if the diff shows one, and **ask whether it should be investigated
     further** before going on. A yellow receipt fails CI until fixed or accepted (see 4).
   - **MAJOR** (10% or more): **red-flag it**. The PR must not be merged until the regression is
     fixed, or the user explicitly accepts it; record the reason in the PR's *Performance* section and
     on the receipt's `**Accepted:**` line (replacing its `-`).
   - A **differing checksum** (of the values or of the points found) or **host/device disagreements**
     are flagged whatever the timing says: the sides computed different answers.
   - A **lost** case (ran on the base, skipped or missing on the PR) is red: find out why it no longer
     runs before reading the timings.
   - Before calling something a regression, repeat the flagged cases by hand with more rounds
     (`Scripts/run-performance.sh compare origin/<base> <pr-branch> '<glob>' --rounds 10`); do not
     rerun until a noisy result passes.
4. **CI times nothing; it checks the receipt** (`Performance-receipt` in `.github/workflows/CI.yml`,
   `Performance/CheckReceipt.cmake`, required by `CI-passed`). It fails when the PR changes the library
   or the probes and its receipt has no host section, or when any section present (host, or a device
   one) measured other code (any later code change makes it stale; a later docs-only commit does not),
   measured an outdated base (rebase and record again), or is not green and has no reason on its
   `**Accepted:**` line under the section's heading (a yellow verdict, a minor regression or a changed
   checksum, needs one too); a pass on an accepted verdict shows as a warning. Write an accepted reason
   into the receipt only when the user explicitly accepts that verdict, and give it as the user did:
   CI cannot tell who wrote it, and git history shows only that it was pushed from the user's account.

**Coverage: how wide the probes are, and when to add to them.** The probes cover each public
structure where its cost lives: its build, its queries, and any in-place update (a refit), in both
precisions, on the host and, where the operation runs on a device, as a `hostdevice` pair. When a PR adds a public structure or query, or a device path for an existing
one, **it adds the cases in the same PR** (a new folder under `Performance/` with a `main.cpp`, or
cases in its component's probe), and the PR's comparison then lists them as *new*. Rules:
- One operation on one input per case, at the sizes users meet (one repetition takes at least a few
  milliseconds), with a `--smoke` size that runs in well under a second.
- Spell out every parameter that selects code (`K`, `W`, precision, construction method) instead of
  using the `Default*` functions, and record it with `param()`; record a `checksum()` that depends on
  every result, and `poolBytes()` where there is a pool. A case computing the same answers as another
  by other means (fused and through a tape) names it with `relativeTo()`, and the comparison notes the
  ratio of their times (`tape/fused 2.78x`).
- Add a case rather than change an existing one: a changed case cannot be compared with its past.
- Keep the whole set to about ten minutes of a comparison with its defaults on a desktop; a case
  that repeats another's code path (the same kernel at another size) must have shown different
  behaviour to earn its place.
- Cross-library comparisons (fcpw, nanoflann, ...) do not belong here; they live in the
  EBGeometry-benchmarks repository.

## Branching, pull requests, and other spin-offs

**You are never allowed to automatically create new branches, submit pull requests, create git
worktrees, or take any similar spin-off action without first asking the user and obtaining explicit
approval.** Any spin-off must obtain explicit user approval. This holds even when it looks like the
obvious next step — for example, isolating an unrelated change onto its own branch, or opening a PR
once work is done: surface the option and let the user decide, do not just do it. Committing to the
branch you are *already on* (once the user has asked for that work) is fine; it is the creation of
new refs and the outward submission of PRs that always needs a green light first.

When the user *does* ask you to open a PR, **always open it as a draft** (`gh pr create --draft`) —
never open a PR ready for review. The user promotes it out of draft themselves when they judge it
ready; this is also what gates the `.rst` documentation audit described above (which begins once
the PR is no longer a draft). If a non-draft PR was already opened, convert it back with
`gh pr ready --undo <number>`.

When opening a PR, **always use the repository's pull-request template**
(`.github/pull_request_template.md`) — its `# Summary` sections (`Background`, `Solution`,
`Side-effects`, `Alternative solutions`, `Performance`) followed by the `Reviewer checklist`. The
`Performance` section carries the comparison report (see "Vetting every pull request for
performance" above). Fill in the prose
sections (this is the "PR note") but **leave every checklist item unchecked** (`- [ ]`) — the
checklist is for a human reviewer to complete, not you. If a PR was already opened with a body that
does not follow this template, edit the PR body to conform to it.

## A few non-obvious things worth knowing

- **Precision is a compile-time parameter, not a runtime one.** Nearly everything is templated on
  a floating-point type `T`; the library itself has no notion of a "current" precision. Examples
  read `EBGEOMETRY_PRECISION` as a preprocessor define to pick `T` via `using T =
  EBGEOMETRY_PRECISION`. `float` support is real but was, until recently, completely untested —
  `Tests/InstantiateAll.cpp` and the `Examples-FloatPrecision` CI job now cover it.
- **Two runtime checks, one per job.** `EBGEOMETRY_REQUIRE(cond, "Class: format", args...)` is always
  on and guards what a caller controls, once per object built (constructor arguments, sizes, memory
  resources); `EBGEOMETRY_EXPECT(cond)` guards internal invariants and hot paths and is opt-in (below).
  The file readers never abort on a bad file: they throw `Parser::ParseError` (file, line, reason),
  including for meshes that cannot form a half-edge surface or that fold back onto themselves
  (`Soup::findTopologyDefect`/`findFoldedFeature`); holes are still allowed. A trailing
  `Parser::OnDefect::Warn` argument on every mesh-building `readInto*` and on
  `PolygonSoup::convertToDCEL`/`convertToTriangles` loads such a mesh with a
  warning instead (a face that visits a vertex twice still throws). Death tests for
  `REQUIRE` checks use `abortsWith()` from `Tests/TestDeath.hpp` and run in every build.
- **Device code needs no special compiler flags, and must stay that way.** Under `Source/`, never use
  `std::min`/`std::max`/`std::clamp`/`std::numeric_limits`/`std::array`; use `Math::min`/`max`/`clamp`/
  `Limits<T>` (`EBGeometry_Math.hpp`) and `Array<T, N>` (`EBGeometry_Array.hpp`). Inside an
  `EBGEOMETRY_HOST_DEVICE` function, call no `std::` function except device math (`std::sqrt`,
  `std::abs`, `std::isfinite`, ...). nvcc rejects the others without `--expt-relaxed-constexpr`, which
  users are not required to pass, and HIP accepts them, so only `Scripts/CheckDeviceMath.py` (a
  pre-commit hook and CI step) and the advisory CUDA lane catch a violation.
- **Default K and W are 4, with one build-level exception.** `BVH::DefaultBranchingRatio<T>()`,
  `TriangleSoA::DefaultWidth<T>()` and `PointSoA::DefaultWidth<T>()` never read ISA macros, so a type
  spelled with them is identical in every translation unit and GPU compile pass. The exception is
  `EBGEOMETRY_HOST_TUNED_DEFAULTS`: in a host-only build that defines it for every translation unit,
  `TriangleSoA::DefaultWidth` returns `TriangleSoA::HostWidth` (K and the point width stay 4; widening
  them measured no faster). The headers `#error` on it in a CUDA/HIP translation unit (keyed on
  `EBGEOMETRY_CUDA`/`EBGEOMETRY_HIP`, never `EBGEOMETRY_DEVICE_COMPILE`), and the CMake option of the
  same name is ON for a top-level non-GPU build, so the `release`/`release-test` presets test with
  W = 8 for `float`. The ISA-tuned values (`BVH::HostBranchingRatio`, `TriangleSoA::HostWidth`,
  `PointSoA::HostWidth`) must never be used for a type device code also sees. See "Branching factor
  and SIMD width" in `ConfigurationOptions.rst` and `Examples/HostTuning`.
- **`EBGEOMETRY_EXPECT()` assertions are opt-in** (`EBGEOMETRY_ENABLE_ASSERTIONS`, ON in `debug`/
  `debug-san`, OFF in `release`/`release-test`) and are the primary way internal invariant
  violations (e.g. a dangling half-edge, a malformed mesh) surface during development; they compile
  to nothing in Release.
- **DCEL topology is index-based, not pointer-based.** Every cross-reference (edge→pair edge,
  edge→face, vertex→outgoing edge, face→half-edge, etc.) is a plain `uint32_t` index into the
  owning `DCEL::MeshT`'s own vertex/edge/face arrays, sentinel `UINT32_MAX` for "unset" — never a
  `weak_ptr`/`shared_ptr`. This is what makes `VertexT`/`EdgeT`/`FaceT` trivially copyable
  (`static_assert`-checked in their headers), a prerequisite for the GPU port's `memcpy`-based
  mirroring. Any member function that needs to resolve one of these indices back into an actual
  vertex/edge/face therefore takes the owning `MeshT` as an explicit parameter (e.g.
  `edge.getVertex(mesh)`, `face.signedDistance(point, mesh)`) — introducing a *new* back-reference
  field should follow the same pattern (an index member, resolved via an explicit mesh parameter),
  not a cached pointer, which would only be valid in the address space it was set in and would
  need re-patching after every host-to-device mirror.
- **`MeshSDF` holds its source `DCEL::MeshT` descriptor as well as the packed BVH, and owns no
  memory; the `Pool` is what must stay alive.** `MeshSDF` is a trivially copyable value type: its
  `MeshT` and its `PackedBVH` (of `uint32_t` face ids) are both descriptors resolving against the
  one `Pool` passed to the constructor. Every leaf test reads the face from the mesh, so the mesh is
  the only copy of the geometry (a flip or a reconcile-and-refit after the build is seen). Copying a `MeshSDF` copies descriptors, not data (use `deepCopy(Pool&)`
  for independent storage), and nothing is reference-counted, so the caller must keep the `Pool`
  alive for as long as the `MeshSDF` or any copy or `rebasedView()` of it is used (and, on a device,
  the mirrored pool too). The same holds for `FlatMeshSDF` (mesh only), `TriMeshSDF` (BVH only; its
  `TriangleAoSoA` leaves are self-contained), `PointCloudBVH`, `PointCloudHashGrid` and the BVH
  unions. Each of these is a handle: it derives from `PoolHandle` (the one `PoolLocation`), and
  its `Layout` (offsets only) is what nested storage holds, so a union stores a `TriMeshSDF` as
  `TriMeshSDF::Layout` and rebuilds it with its own location as it evaluates it. A similar wrapper
  should follow the same pattern: derive from `PoolHandle`, hold the layouts of its parts (never a
  second handle, which would carry a second location), offer `rebasedView()`/`deepCopy()`,
  `layout()` and a `(Layout, PoolLocation)` constructor, and document that its pool must outlive it.
- **Mesh elements and points carry no user metadata; an element's id is its index.** The DCEL
  classes, `Triangle`, the mesh SDFs, the parsers, `PointAoSoA` and the point clouds have no `Meta`
  template parameter. The readers number faces in file order (minus removed degenerate faces):
  `readSTL`/`readPLY`/`readOBJ`/`readVTK`/`readIntoPolygonSoup` return one cleaned `PolygonSoup`
  whose face i and vertex i are face i and vertex i of the mesh, with PLY/VTK properties remapped to
  match. `getClosestFace()` on all three mesh SDFs returns that id (device-callable), and
  `Triangle`/`TriangleAoSoA` carry the id of the face a triangle was cut from. A point's cloud index
  is its position in the positions array; `PointCloudBVH` and `PointCloudHashGrid` report it in one
  `PointCloud::Hit` (`EBGeometry_PointCloud.hpp`, with the shared `KBest` set and the brute-force
  references). Per-element user data lives in the caller's own array.
- **Header layout.** `EBGeometry_PackedBVH.hpp` is the only BVH header device code needs;
  `EBGeometry_TreeBVH.hpp` (`TreeBVH`, partitioners, `pack()`) and `EBGeometry_BVHBuild.hpp`
  (`PackedBVH`'s static builds: `build()`, `buildTopDown()`, `buildBottomUp()`,
  `buildClusterSAH()`, and the constructors behind `pack()`) are host-only and include each other. The blend operators and
  BVH unions are in `EBGeometry_Blend.hpp`/`EBGeometry_BVHUnion.hpp`, the host-only expression builder
  (`Expr`, `Union`, `Translate`, ...) in `EBGeometry_Expr.hpp`, the `Tape` that compiles and evaluates
  an expression in `EBGeometry_Tape.hpp` (with `Affine3T` in `EBGeometry_Affine.hpp`), the fused
  operations (`Fused::Union`, ...) in `EBGeometry_Fused.hpp`, `PrimitiveRef` (the deciding leaf
  `closestPrimitive()` returns) in `EBGeometry_PrimitiveRef.hpp`, each operation's arithmetic, shared by
  the tape and the fused operations, in `EBGeometry_CSGOps.hpp`, `EBGeometry_CSG.hpp` including all of
  them, and `operator<<` for `Vec3T`/`AABBT` in `EBGeometry_StreamOperators.hpp`. `EBGeometry_BVH.hpp`, `EBGeometry_CSG.hpp` and `EBGeometry.hpp`
  include the split headers, so older includes keep working.
- **Composition goes through the tape or the fused operations, and both must stay device-safe.**
  There is no `ImplicitFunction` or virtual CSG layer: shapes (`isShape<S>`: trivially copyable, with
  `signedDistance()` and `computeBoundingVolume()`) are composed into an `Expr` and compiled into a
  `Tape<T, UserPrimitives...>`, which stores one pool array per primitive type and dispatches on a
  type index; or, when the structure is fixed at compile time, with the `Fused::` functions into a
  `Fused::Node<Op, Operands...>`, itself a shape (a pool handle, following the wrapper rule below,
  when an operand lives in a pool). Both take each operation's arithmetic from `CSGOps`, so they give
  the same values (to rounding: the compiler may fuse multiply-adds differently in the two, so a
  Release build differs in the last bit): change an operation there, never in one of the two alone,
  and keep `TestFused`'s comparison with the tape passing. `CSGOps` works on a plain value or on a
  `CSGOps::Tagged` one (for the tape's `closestPrimitive()`; a fused composition is one shape with one
  surface, a single leaf of a tape, and has none); keep each function's plain-`T` code written as it is,
  with the tagged handling in `if constexpr` branches or a `Tagged<T>` overload. An equivalent rewrite
  costs on both sides: of the plain Blur loop, a 16-evaluation float tape kernel went from 168 to 255
  registers; of `pairInsert()`/`pairBlend()` alone, GCC stopped inlining the float tape's primitive
  dispatch, and host tapes without a smooth union became up to 37% slower. After any change on the
  tape's path, compile a float kernel calling `signedDistance()` once and one calling it 16 times in a
  loop and check `ptxas -v` still reports the registers dev's do, and compare the sizes of the host
  evaluator's out-of-line functions (`nm -C -S` on `Performance/Tape/main.cpp` built `-O3
  -march=native`) with the base's: a function that newly appears or changes size there moves the
  receipt. A shape type that is not built in (a BVH union, a mesh SDF with a non-default K,
  another tape, a user type) must be listed in `UserPrimitives`. The evaluator has no recursion: a
  sub-program call (Blur, Mollify, FiniteRepetition, BVH union operands) goes through the `evaluate<D>` call-depth
  template, bounded by `Tape::MaxCallDepth`, and the value/point stacks are fixed arrays; keep it
  that way (no recursion, function pointers or virtual calls), since the device evaluator builds on
  it. `evaluate<D>` and the pool-resident primitive dispatch (`evaluateHandle`) are
  `EBGEOMETRY_DEVICE_NOINLINE`: HIP inlines every device function, and `evaluate<D>` calls the level
  below from many places, so fully inlined the copies multiply with the depth (an optimised device
  compile of `TestTape` did not finish). Keep them out of line, and mark any new function called that
  way the same. What multiplies nothing is inlined, for speed on a device (#221): the body
  `evaluateBody<D>` into `evaluate<D>`, and into `signedDistance()` for a tape of call depth 0; and an
  analytic primitive into the level that reads it. A new operation needs an `ExprDetail::Kind`, its
  box and distance-quality rule in the builder, its arithmetic in `CSGOps`, clauses in
  `TapeDetail::Op` and a case in `Tape::evaluateBody`, and a `Fused::` operation and function.
- **Sign convention: negative inside, positive outside, everywhere in EBGeometry.** Every shape, mesh
  SDF, union and tape keeps it. Code that needs the opposite sign (AMReX and Chombo read negative as
  fluid) flips it itself, in its own adapter (`return -tape.signedDistance(p);`); never put a
  `Complement` into an EBGeometry expression to suit the consumer.
- **Every CMake preset gets its own `build/<preset-name>/` directory** (see `CMakePresets.json`'s
  `binaryDir`) specifically so switching presets can't silently reuse a stale `CMakeCache.txt` from
  a different configuration.
- **`TriMeshSDF` fan-triangulates polygon faces**; `MeshSDF`/`FlatMeshSDF` use the (planar, convex)
  polygon faces directly. Fan triangulation of a planar convex face gives the same signed distance.
  The file readers remove zero-area faces and repair the T-junctions they fill
  (`Soup::removeDegeneratePolygons`); `Tests/data/tetrahedron_sliver.stl` and `cube_quads.obj` cover
  both.
