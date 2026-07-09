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
`ThirdParty/` holds illustrative (not CI-tested) examples coupling EBGeometry to third-party
application codes (AMReX, Chombo), and `Docs/Sphinx/` + `Docs/doxygen.conf` are the two
documentation systems.

## Cloning

```bash
git clone --recurse-submodules git@github.com:rmrsk/EBGeometry.git
```

The `common-3d-test-models` submodule provides mesh files (`armadillo.obj`, `cow.obj`, ...) used
by the `MeshSDF`/`CSGUnion` examples. If cloned without
`--recurse-submodules`, run `git submodule update --init` afterward. It is **not** needed to build
or test the library itself — only to run those two examples with their default (no-argument)
input.

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

ctest --preset debug            # unit tests only, ~0.3 s
ctest --preset examples         # run every example via ctest, several minutes in Debug mode
```

| Preset          | Build type | Assertions | SIMD | Tests/examples built |
|-----------------|------------|------------|------|-----------------------|
| `debug`         | Debug      | ON         | none | ON |
| `debug-san`     | Debug      | ON         | none | ON, + ASan/UBSan |
| `release`       | Release    | OFF        | avx  | OFF |
| `release-test`  | Release    | OFF        | avx  | ON |

`EBGEOMETRY_SIMD` (`none|sse41|avx|avx512`) and `EBGEOMETRY_ENABLE_ASSERTIONS` are ordinary CMake
cache variables if you need a combination not covered by a preset.

## Testing

```bash
ctest --preset debug                 # 130+ unit tests (Catch2), sub-second
ctest --preset debug-san             # same, under AddressSanitizer + UBSan
ctest --preset examples              # every Examples/* program, run to completion
ctest --preset release-test          # unit tests + examples, optimised build
```

Test sources live in `Tests/Test*.cpp` (one file per component: `TestVec`, `TestBoundingVolumes`,
`TestDCEL`, `TestBVH`, `TestSTL`/`TestPLY`/`TestOBJ`/`TestVTK`, ...); fixture files are in
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
   `Tests/TestingLocally.rst`-style coverage table, matching how existing sibling classes are
   documented.
4. **If you changed a function's signature** (added/removed/renamed a parameter, changed a return
   type), update its Doxygen `@param`/`@tparam`/`@return` comment in the same header -- `doxygen
   doxygen.conf` (`WARN_AS_ERROR = FAIL_ON_WARNINGS`) will catch a genuinely missing `@param` for a
   new parameter, but won't catch a stale description that no longer matches what the parameter
   does, or a `Docs/Sphinx/source/*.rst` page that still describes the old signature.
5. **Rebuild the Sphinx HTML docs** (see below) and check for build warnings -- a broken internal
   cross-reference (`:ref:`/`:numref:`/`:eq:`) will not stop the build, but usually surfaces as a
   warning.

The `check-docs` pre-commit hook (`Scripts/CheckDocs.py`, manual stage) enforces the ban
mechanically: it fails if any `.. literalinclude::` directive exists anywhere under
`Docs/Sphinx/source/`. Treat it as a floor, not a replacement for the steps above -- it only
catches the banned directive itself, never "this page no longer describes what the code does,"
which needs the manual read-through above.

## Reproducing CI locally / before pushing

```bash
Scripts/run-all-checks.sh
```

Runs every pre-commit hook (default and manual stage — formatting, REUSE/license headers,
codespell, Doxygen, clang-tidy, a debug-preset compile check, the advisory doc/source
cross-reference check, Sphinx HTML/PDF) followed by all four CMake presets' test suites. This is
the same check set `.github/workflows/CI.yml` runs, just local and in one command; expect it to
take several minutes. `pre-commit install` is only needed if you want the default-stage hooks
(formatting, license, codespell, Doxygen) to run automatically on `git commit`; the manual-stage
hooks (clang-tidy, the debug build, Sphinx) only run via this script or explicit
`pre-commit run --hook-stage manual`.

`.clang-tidy` and `.pre-commit-config.yaml` define the static-analysis/formatting rules; CI
(`.github/workflows/CI.yml`) runs the same hooks plus a much larger build/test matrix (multiple
compilers, SIMD levels, precisions, sanitizers, both Debug and Release) that isn't practical to
reproduce byte-for-byte locally.

## Branching, pull requests, and other spin-offs

**You are never allowed to automatically create new branches, submit pull requests, create git
worktrees, or take any similar spin-off action without first asking the user and obtaining explicit
approval.** Any spin-off must obtain explicit user approval. This holds even when it looks like the
obvious next step — for example, isolating an unrelated change onto its own branch, or opening a PR
once work is done: surface the option and let the user decide, do not just do it. Committing to the
branch you are *already on* (once the user has asked for that work) is fine; it is the creation of
new refs and the outward submission of PRs that always needs a green light first.

When the user *does* ask you to open a PR, **always use the repository's pull-request template**
(`.github/pull_request_template.md`) — its `# Summary` sections (`Background`, `Solution`,
`Side-effects`, `Alternative solutions`) followed by the `Reviewer checklist`. Fill in the prose
sections (this is the "PR note") but **leave every checklist item unchecked** (`- [ ]`) — the
checklist is for a human reviewer to complete, not you. If a PR was already opened with a body that
does not follow this template, edit the PR body to conform to it.

## A few non-obvious things worth knowing

- **Precision is a compile-time parameter, not a runtime one.** Nearly everything is templated on
  a floating-point type `T`; the library itself has no notion of a "current" precision. Examples
  read `EBGEOMETRY_PRECISION` as a preprocessor define to pick `T` via `using T =
  EBGEOMETRY_PRECISION`. `float` support is real but was, until recently, completely untested —
  `Tests/InstantiateAll.cpp` and the `Examples-FloatPrecision` CI job now cover it.
- **`EBGEOMETRY_EXPECT()` assertions are opt-in** (`EBGEOMETRY_ENABLE_ASSERTIONS`, ON in `debug`/
  `debug-san`, OFF in `release`/`release-test`) and are the primary way internal invariant
  violations (e.g. a dangling half-edge, a malformed mesh) surface during development; they compile
  to nothing in Release.
- **DCEL topology uses `weak_ptr` for back-references** (edge→pair edge, edge→face, vertex→
  outgoing edge, etc.) with the `DCEL::MeshT`'s own vertex/edge/face vectors as the sole
  `shared_ptr` owners. Introducing a *new* back-reference field as a `shared_ptr` will very likely
  create a reference cycle that leaks (invisible without a leak-checking sanitizer, since nothing
  crashes) — always use `weak_ptr` + `.lock()` for anything that points "backward" in the mesh
  graph.
- **`MeshSDF` retains its source `DCEL::MeshT`, not just the packed BVH.** If you write a similar
  wrapper around a DCEL mesh, remember that a `PackedBVH` of faces holds `shared_ptr<const
  DCEL::FaceT>`s that transitively `weak_ptr`-reference the mesh's edges/vertices — nothing keeps
  the mesh itself alive unless something holds an explicit `shared_ptr<DCEL::MeshT>`.
- **Every CMake preset gets its own `build/<preset-name>/` directory** (see `CMakePresets.json`'s
  `binaryDir`) specifically so switching presets can't silently reuse a stale `CMakeCache.txt` from
  a different configuration.
- **`TriMeshSDF` requires an all-triangular mesh**; `MeshSDF`/`FlatMeshSDF` accept arbitrary
  (planar, convex) polygon faces. The `Tests/data/dodecahedron.*` fixture (20 vertices, 36
  triangulated faces, exercised by `Tests/TestBVH.cpp`) works for both, since its pentagonal faces
  were pre-triangulated when generated.
