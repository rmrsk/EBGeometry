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

Sphinx is pinned to `sphinx==5.0.0` (see `.pre-commit-config.yaml`) for compatibility with
`sphinxcontrib-bibtex`; installing/using a different Sphinx version standalone can produce stale
or incompatible `Docs/Sphinx/build/` doctree caches. **If a Sphinx build throws an internal error
in `sphinx.environment.collectors.toctree`, or `rm -rf Docs/Sphinx/build` fails with "Directory not
empty," check for a stray `sphinx-autobuild` (or other long-running `sphinx-build`) process first
(`pgrep -fa sphinx`) — a forgotten one rebuilding into the same output directory in the background
has caused exactly this before.**

## When you change code, check whether the docs still agree with it

Sphinx docs and Doxygen comments describe the code but aren't checked by the compiler, so they
silently drift out of sync with it. This has already happened once in this repository: adding a
field to `MeshSDF`'s constructor shifted line numbers in
`Source/EBGeometry_MeshDistanceFunctionsImplem.hpp`, which silently made
`Docs/Sphinx/source/ImplemBVH.rst`'s `literalinclude` `:lines:` range cite the wrong span --
nothing failed to compile, no test failed, and the only reason it was caught was a manual full
Sphinx rebuild and a careful read of the rendered excerpt.

Before treating a code change as finished, look at what you actually staged/changed
(`git diff --staged --name-only`, or `git diff HEAD --name-only` if not yet staged) and, for each
changed `Source/*.hpp` file:

1. **Find `literalinclude`s pointing at it**: `grep -rn "literalinclude.*<filename>" Docs/Sphinx/source/`.
   For each match, check whether its `:lines:`/`:start-after:`/`:end-before:` range still bounds
   the *intended* snippet after your edit -- adding, removing, or reordering even one line upstream
   of a fixed `:lines: N-M` range silently shifts what gets excerpted. Prefer `:start-after:`/
   `:end-before:` (anchored to text, not line numbers) over `:lines:` when editing or adding a new
   literalinclude, since it survives future line-number drift; leave existing `:lines:` ranges as
   `:lines:` unless you're touching them anyway.
2. **Rebuild the Sphinx HTML docs** (see below) and check both for build warnings (a
   "non-whitespace stripped by dedent"-style warning means the range no longer aligns with a code
   block) and, if the affected page renders a code snippet a reader is meant to learn from, that the
   *rendered* excerpt is still the right one -- a shifted range can still produce syntactically
   plausible C++ that is simply the wrong function.
3. **Grep for renamed/removed identifiers** in prose, not just code blocks: if you renamed a class,
   method, or parameter, `grep -rn "<oldName>" Docs/Sphinx/source/ Docs/mainpage.md README.md
   CLAUDE.md` to catch references outside literalincludes (which update automatically) that don't.
4. **If you added a new public class/function**, add it to `Tests/InstantiateAll.cpp` (for
   clang-tidy/compile coverage under both precisions, per the existing convention) and check whether
   it deserves a mention on the relevant `Docs/Sphinx/source/Implem*.rst` conceptual page and/or a
   row in a `Tests/TestingLocally.rst`-style coverage table, matching how existing sibling
   classes are documented.
5. **If you changed a function's signature** (added/removed/renamed a parameter, changed a return
   type), update its Doxygen `@param`/`@tparam`/`@return` comment in the same header -- `doxygen
   doxygen.conf` (`WARN_AS_ERROR = FAIL_ON_WARNINGS`) will catch a genuinely missing `@param` for a
   new parameter, but won't catch a stale description that no longer matches what the parameter
   does.

The `check-docs` pre-commit hook (`Scripts/CheckDocs.py`, manual stage) automates a coarse version
of step 1: it diffs against `main` and prints every `literalinclude` whose referenced file changed.
Treat it as a starting point, not a replacement for the above -- it only flags "this file changed,"
never "this range is now wrong," which is precisely the failure mode that bit this repo before.

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
