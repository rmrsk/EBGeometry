# Pre-tape audit

A review of the whole codebase at the pre-tape checkpoint (`dev` at 63739a0, after #150–#152),
covering code quality **and** code design. The point of the checkpoint is to fix what is wrong, and
to settle what the tape will be built on, before the tape freezes it in place.

Method: six parallel area reviews, each told to verify claims by reading the code or by running a
scratch program. Every BLOCKER, and every MAJOR marked "reproduced" below, was re-run independently
against the current tree before being recorded here. Findings keep their per-area numbering
(`MEM-1`, `BVH-3`, ...) so they can be cited from issues and PRs.

| Prefix | Area |
|--------|------|
| `MEM`  | Memory model (`MemoryResource`, `Pool`, `PODVector`), GPU macros, `Vec` |
| `BVH`  | `TreeBVH`, `PackedBVH`, partitioners, SFC, bounding volumes |
| `MESH` | DCEL, `FlatMeshSDF`/`MeshSDF`/`TriMeshSDF`, triangles and SoA kernels, parsers |
| `CSG`  | Implicit functions, CSG, transforms, analytic shapes, octree bounding volumes |
| `PC`   | `PointCloudBVH`, `PointCloudHashGrid`, point SoA, `Octree`, `Random`, `SimpleTimer` |
| `QA`   | Tests, examples, integrations, build system, CI, documentation |

Severity: **BLOCKER** gives wrong results or crashes in normal use; **MAJOR** is a real defect or a
design problem that should be settled before the tape; **MINOR** should be fixed but does not gate
the tape; **NIT** is cosmetic.

---

## 1. Executive summary

The port has produced a sound *query* core: trivially copyable descriptors over offset-based pool
storage, one stack-based `pruneTraverse`, value-type shapes and mesh SDFs, and single-type BVH
unions that run on the device. The problems sit on either side of that core.

**Below it, there are wrong answers.** Nine defects produce wrong geometry or crash on
ordinary input. All were reproduced:

| ID | Defect | Reproduction |
|----|--------|--------------|
| BVH-1  | SFC builds (`Build::Morton`/`Nested`) pad leaves by duplicating the last one; `BVHSmoothUnionIF` then blends a primitive with itself | 1228/20000 wrong (Morton), 583/20000 (Nested), error up to s/4 |
| MESH-1 | A zero-area face gets a NaN normal, which poisons neighbouring pseudonormals | 3015/27000 grid signs wrong on a tetrahedron with one sliver |
| MESH-2 | Binary STL with per-facet colour attributes is split by colour; only one group is returned | 2 of 4 faces read |
| PC-1   | `PointCloudBVH` aborts on a few hundred coincident points (midpoint split never separates them) | 500 coincident points → depth 147, abort |
| CSG-1  | `Scale` with a negative factor turns the shape inside out | `Scale(-2)` gives +1 at the mapped centre |
| CSG-2  | `Mollify`'s kernel is inside-out (negative centre weight) | n=3: w(0) = −0.10 |
| CSG-3  | `RoundedBoxSDF` is not an SDF inside; `Annular`/`Offset` of it build the wrong solid | −0.1 at the centre of a 2×2×2 box (exact −1.1) |
| CSG-4  | `ExpMinOp` overflows to ±inf ~100·s from the surface in `float` | `ExpMin<float>(200,200,1) = inf` |
| PC-2   | Hash-grid query far from the cloud overflows a float→int cast (UB), wrong neighbour | query at x=1e8 returns x=0.04 instead of x=1.0 |

A tenth, BVH-6 (`SphereT(vector<SphereT>)` does not enclose its inputs; 49% of random sets), is
latent because no in-tree BVH uses `SphereT`.

These must be fixed **before** the tape, for a reason beyond the bugs themselves: the tape's
test suite will take today's primitives as its reference values, and would enshrine these errors.

**Above it, the design has not caught up with the port.** Three reviewers converged, independently,
on the same three structural issues:

1. **Descriptors locate themselves** (MEM-1, MEM-2, BVH tape notes, MESH tape notes). Every
   pool-resident class carries its own `PoolControl*`/base and picks between them by compilation
   pass. The `PoolLocation`/`relocatedTo`/`IsPoolResident` machinery from #152 works, but it is a
   workaround for that choice, it is duck-typed, and it breaks managed/mapped memory on the host.
   The tape is the deepest nesting of pool-resident objects yet.
2. **The build side is duplicated and inconsistent** (BVH-2/3/4/7/8, PC-6, MESH-14). There are
   five `PackedBVH` constructors plus `TreeBVH`, about seven copies of the same sort/split/pack logic,
   four traversal loops, and a `Build` enum that means a different algorithm and leaf size in each
   consumer. None of it can build on the device.
3. **Types multiply** (MESH-6/8/9, PC-4/14, BVH-5). The `Meta` template parameter is never
   populated but is threaded through every mesh type. K and W defaults depend on ISA macros, so the
   "same" SDF is a different type in a `.cpp` and a `.hip` file. There are three mesh SDFs, two
   point-cloud classes with divergent interfaces, and eight copies of the point–triangle kernel.
   In the tape each distinct type is another opcode.

**The safety net is thinner than it looks** (QA-5/6/7/13). The host suite is solid: 598 ctest cases
pass with both precisions. But no device code executes anywhere: every `[gpu]` case SKIPs on its
first line, and the GPU lanes are `continue-on-error` and outside `CI-passed`, so a PR that breaks
device compilation stays green. Six of the twelve analytic shapes have no host test of their
formula, and mesh-SDF signs are tested only on convex fixtures. Those formulas are the future
opcodes, and the tape's validation needs them as independently checked references. Around the
library, 8 of 9 integrations no longer compile (QA-1), and the CMake target injects `-mavx` into
every consumer by default (QA-2).

Separately, **the virtual CSG/transform layer can't be looked into** (CSG-5). Its nodes hide their
children and parameters, and have no type tag or visitor, so no compiler can lower an existing
`shared_ptr<ImplicitFunction>` graph into tape clauses. The tape needs its own front-end; that is a
decision to make now, not during tape implementation.

The recommended order (section 5) is: correctness fixes first, as small independent PRs with
regression tests; then the three foundation refactors; then retirements and API cleanup; then tape
design. Section 4 records the decisions the maintainer took.

---

## 2. Cross-cutting design review

### 2.1 Where the pool location lives

**Today.** `MeshT` and `PackedBVH` each implement `m_control` + `m_base`, `base()`, `attachTo`,
`isAttachedTo`, `location`, `relocatedTo` and the `rebasedView` checks, with small incidental
differences (`const Pool&` vs `Pool&`, `void*` vs `const void*`). Six more classes forward to them.
On the host pass `base()` reads `m_control->m_base`; on the device pass it reads `m_base`.

**Consequences.**
- A view onto a managed or mapped mirror nulls `m_control`, and the host pass then dereferences it:
  a Debug assert, a Release segfault (MEM-2, reproduced with a fake host+device-accessible resource).
  Building directly in managed memory, which the `MemoryResource` docs advertise, is impossible.
- A pool-resident object *stored inside another pool* holds a host control pointer that is dangling
  or meaningless where it is read. #152 solved this by relocating each primitive at evaluation time,
  detected via `IsPoolResident` ("has a `relocatedTo` member"). A future type that forgets
  `relocatedTo` compiles and reads garbage.
- `MeshSDF` (136 B) carries two `PoolLocation`s that must agree, unchecked.
- The port's own rule is "store an offset, not a pointer; the base is supplied by the caller". The
  descriptors half-follow it.

**Recommendation (MEM-1).** Split each pool-resident class into:
- a **layout**: offsets and sizes only, trivially copyable, with HOST_DEVICE queries taking `base`
  explicitly; nested storage (union primitives, tape payloads) holds layouts;
- a generic **handle** `PoolHandle<Layout>` = layout + one `PoolLocation`, which owns `base()`,
  `rebasedView`, `isAttachedTo`, and the lineage checks. Only top-level user objects are handles.

Location is then decided by value (control or snapshot), not by pass, which fixes MEM-2 in the same
change. `relocatedTo` and `IsPoolResident` disappear. `deepCopy` stays per class, or becomes generic
if layouts expose a `forEachArray` visitor. A smaller alternative is to give `PoolLocation` the
behaviour and hold it as a member, which removes the duplication but keeps the relocation workaround.

### 2.2 One BVH builder, one layout, one traversal

**Today** (BVH-2/3/4/7/8):
- `Build::Morton` means `TreeBVH::bottomUpSortAndPartition` in `MeshSDF` and `TriMeshSDF`, and the
  padded `PackedBVH` SFC constructor in `BVHUnionIF`.
- Leaf-size rules differ: `< K` in `DefaultLeafPredicate`, `≤ maxLeafGroups·W` in `TriMeshSDF`,
  and `ClusterSpec` leaves hold up to (K−1)·`maxClusterSize`. (Reassessed later as a tuning and API
  gap, not an inconsistency to unify; see BVH-2 in section 6.)
- `ClusterSAH`, `Midpoint` and `Hilbert` can't be reached through `Build`.
- Partitioners are `std::function`s over `shared_ptr` lists that allocate per node. `LeafPredicate`
  takes a `TreeBVH&`, so even the "direct" top-down constructor builds probe `TreeBVH`s.
- There are four traversal loops (`TreeBVH::traverse`, `PackedBVH::traverse`, `pruneTraverse`,
  `PointCloudBVH`'s DFS). `TreeBVH::traverse` has no callers and reads a destroyed `shared_ptr`.
- Every node box is stored twice, and a zeroed SoA row is reserved for every leaf (~75% of rows).
- `PackedBVH` can only be constructed from host `std::vector`s, and its validation runs on host
  memory, so nothing can be built on the device (PC-6).

**Recommendation.**
- **One host builder** over a (centroid, AABB) scratch array, with index-range partitioners as
  template parameters, returning `{nodes, leafOrder}`. Delete `TreeBVH`, `pack`/`packWith`,
  `PrimAndBV`/`PrimitiveList`, and both `traverse()` functions with their seven `std::function`
  aliases. `MeshSDF::getClosestFaces`, the only `traverse()` caller, becomes a `pruneTraverse` use.
- **One build-spec value type** (strategy, curve, max leaf size) interpreted identically by every
  consumer, replacing the `Build` enum. *Not adopted:* `BVH::Construction` stays the method
  selector (17c), and leaf size is set per method rather than with one universal bound (BVH-2).
- **Empty child slots** (sentinel child, inverted box ⇒ +inf distance ⇒ pruned at push) instead of
  duplicate-leaf padding. This is the BVH-1 fix and costs no traversal change.
- **A wide-node layout**: child SoA boxes, child indices and per-child leaf ranges, no per-node
  `m_bv`, SoA rows for interior nodes only, explicit leaf marking, `requireWellFormed` on every
  build.
- **An adopt-pool-resident-arrays constructor** with a documented node-count upper bound, so a
  device builder (Morton + radix sort, for point clouds first) and the tape can produce BVHs
  without a host round trip.
- **A device stack policy per K** (BVH-5): at K=16 the 64-entry device stack allows only depth 5,
  and a 200k-primitive skewed set already needs 6–7. Shrink `StackEntry` (uint32 index + float
  distance) and/or size the stack per K; document `DefaultBranchingRatio` as host tuning only.
- **Degenerate-extent fallback** to an object-median split (PC-1), so coincident primitives can't
  blow up depth in any builder.

### 2.3 Fewer types, fixed device layouts

**Today.**
- `Meta` is threaded through vertex, edge, face, mesh, triangle, AoSoA, `MeshSDF`, `TriMeshSDF` and
  every parser entry point. No loader populates it (MESH-6), so `getClosestTriangle().metaData` is
  always `Meta{}` for parsed meshes.
- `DefaultWidth<T>()`/`DefaultBranchingRatio<T>()` depend on `__AVX__` and friends, and CMake gives
  SIMD flags to CXX but not CUDA/HIP TUs. `TriMeshSDF<float, Meta>` is `<8,8>` in a `.cpp` and
  `<4,4>` in a `.hip` file (MESH-9). The AMReX integration spells K and W out for this reason.
- There are three mesh SDFs. `FlatMeshSDF` passes through to `MeshT::signedDistance`. `MeshSDF` packs
  stale *copies* of face normals, so `mesh.flip()` after building gives face-interior and
  edge/vertex regions opposite signs (MESH-4, reproduced). Because its polygons must be planar and
  convex, fan triangulation is exact, so `MeshSDF` offers nothing `TriMeshSDF` doesn't (MESH-8).
- There are eight point–triangle kernels: three scalar, five SIMD copies (~1100 lines).
- The two point-cloud classes claim a "drop-in interchangeable" interface but have different `Hit`
  types, constructors, device status and bounds checking, with ~150 duplicated lines (PC-4).
- `PointSoA::DefaultWidth` and `TriangleSoA::DefaultWidth` are copies of each other, and
  `PointSoAT`/`TriangleSoAT` vs `PointAoSoA`/`TriangleAoSoA` naming disagrees (PC-14).

**Recommendation.**
- Replace `Meta` with a fixed `uint32_t` element id per face/lane. Users keep their own metadata
  array indexed by it; parsers expose PLY/VTK face arrays by the same index. This is also the tape's
  planned "winning-primitive index".
- Make device-visible layouts (K, W) fixed per precision, independent of ISA. Let only the host
  evaluation path vary by ISA. One `SIMD::DefaultWidth<T>()`.
- Keep **one** BVH mesh SDF (`TriMeshSDF`, convex polygons fan-triangulated at load). Keep
  `FlatMeshSDF` as a brute-force reference only, with the direct query moved out of `MeshT` so the
  mesh becomes pure data. Retire `MeshSDF` (decision D3).
- Factor one HOST_DEVICE scalar point–triangle kernel and one SIMD template over a thin per-ISA
  wrapper.
- One namespace-level `PointCloud::Hit<T>` (uint32 index, invalid-sentinel for "no match") and one
  HOST_DEVICE k-best insertion helper shared by both point-cloud classes.

### 2.4 The device toolchain contract

Four independent problems share one root: nothing states what device code in this library may use.
- **SIMD intrinsics compiled for the device** (MESH-5, PC-3, found independently by two reviewers).
  Clang's HIP device pass defines `__AVX__`/`__SSE4_1__` when the host has them, so
  `TriangleSoAT` and `PointSoAT` fail to compile for the device under `-mavx`. `PackedBVH` already
  guards with `!defined(EBGEOMETRY_DEVICE_COMPILE)`; the SoA kernels don't. CI passes only because
  its HIP job builds without SIMD, and because CMake strips SIMD flags from HIP TUs, which also
  silently disables host SIMD in any HIP TU.
- **`--expt-relaxed-constexpr`** (MEM-5). `Vec3T` wraps `std::array`, and Source/ uses `std::min`
  32 times, `std::max` 42 times and `std::numeric_limits` 85 times. nvcc rejects all of these in
  device code without the flag, which is set only on the test targets, not the `EBGeometry`
  INTERFACE target.
- **ODR in `Pool::mirror`** (MEM-3, reproduced with a fake `cuda_runtime.h`). A backend `#if` inside
  a non-template inline function gives host and CUDA TUs different definitions. The linker keeps
  one, and if it keeps the host one every host→device mirror aborts.
- **Device assertions vanish under `NDEBUG`** (MEM-6). The device branch of `EBGEOMETRY_EXPECT` is
  `assert`, which Release configurations disable even with assertions requested.

**Recommendation.** Write the contract into PORTING.md's "Porting a class" checklist and enforce it
in CI before the interpreter, which will be the densest device code in the library, is written:
1. No backend `#if` in non-template inline code; put backend variation behind `MemoryResource`
   virtuals.
2. Every SIMD block in a HOST_DEVICE function is guarded by `!defined(EBGEOMETRY_DEVICE_COMPILE)`.
   Add a CI job combining HIP with `EBGEOMETRY_SIMD=avx`.
3. Either propagate `--expt-relaxed-constexpr` on the INTERFACE target, or (preferred, decision D6)
   provide a HOST_DEVICE `EBGeometry_Math.hpp` (min/max/clamp/abs/limits) and store `T m_X[3]` in
   `Vec3T`, banning `std::min`/`std::max`/`std::numeric_limits`/`std::array` in device paths. This
   also retires the hand-rolled `std::clamp` replacements.
4. Device `EBGEOMETRY_EXPECT` becomes `printf` + trap, independent of `NDEBUG`.
5. State a host-vs-device tolerance policy up front: results agree only to rounding (FMA
   contraction, reciprocal division in `Vec3T`), and the index chosen among equidistant candidates
   is unspecified. Otherwise tape comparison tests will be flaky.
6. Stop injecting ISA flags into consumers (QA-2). `EBGEOMETRY_SIMD` defaults to `avx` and is
   applied to the INTERFACE target, so a FetchContent consumer gets `-mavx -mfma -msse4.1` in its
   own targets. That breaks aarch64 (GCC rejects `-mavx`) and SIGILLs on x86 without AVX. Default to
   `none`, or apply the flags only when EBGeometry is the top-level project. `Building.rst` also
   recommends `-march=native` "for maximum portability", which is backwards.
7. Finish the CMake target: `target_compile_features(cxx_std_17)` on the INTERFACE target, and
   either real `install()`/`export()`/`EBGeometryConfig.cmake` or no "local installation" claim in
   the docs (QA-21). Add a "Building for GPUs" section to `Building.rst`; today the GPU build is
   documented only in PORTING.md (QA-4).

### 2.5 Error policy

Checks on user input and on host-side cold paths are inconsistent. Some are always-on
(`requireDepthFits`, the #152 union checks, moved-from pools, `GPU_CHECK`). Others are
Debug-only `EBGEOMETRY_EXPECT`s that let Release continue into UB:
- host allocation failure (MEM-4: Release continues with a null base);
- `freeze` and `rebasedView` preconditions (MEM-7);
- point-cloud constructor size checks (PC-13);
- zero-area normals (MESH-1);
- non-triangle faces silently truncated by `TriMeshSDF` (MESH-13);
- parser failures (MESH-3: empty, truncated, missing or malformed files crash or return silently).

**Recommendation.** Adopt one rule and apply it everywhere:
- `EBGEOMETRY_EXPECT` is for internal invariants on hot paths only.
- User input and one-time host checks are always on (`fprintf` + `abort`, as the unions do).
- Parsers, which are host-only, report failure explicitly: throw, or return `std::optional`. A mesh
  with zero faces is a hard error before any BVH build.

### 2.6 What to retire instead of porting

Several things on PORTING.md's "port" and "loose ends" lists have no remaining justification.
Porting them would mean designing the tape around them.

| Item | Why retire | Replacement |
|------|-----------|-------------|
| `Octree::Node` (PC-5, CSG-13) | `shared_ptr` tree + `std::function` callbacks; its only user is `approximateBoundingVolumeOctree`; traversal order contradicts its docs (PC-17) | a ~30-line explicit-stack subdivision that keeps a running min/max; device-capable |
| `TreeBVH` (BVH-3) | two remaining consumers, both served by the single builder | §2.2 |
| `TreeBVH::traverse`, `PackedBVH::traverse`, 7 `std::function` aliases (BVH-7) | no callers / one caller; UB; `noexcept` + allocation | `pruneTraverse` |
| `SphereT` (BVH-6) | wrong, no consumer, `PackedBVH` hardcodes AABB | — (or fix, if wanted) |
| `MeshSDF` (MESH-8) | nothing over `TriMeshSDF`; stale-copy bugs | `TriMeshSDF` + fan triangulation (D3) |
| `SignedDistanceFunction` (CSG-14) | no implementers; holds the only `normal()` | free `normal(f, p, δ)` next to `approximateBoundingVolumeOctree` |
| SYCL/OpenACC branches, `EBGEOMETRY_DEVICE`/`GLOBAL`/`INLINE`/`ROUTINE` (MEM-11) | fictional (no memory resource, no device-pass macro) and dead | `#error` for unsupported backends |
| `Random`, `SimpleTimer` in `EBGeometry.hpp` (PC-16) | used only by Examples; pull `<random>`/`<chrono>` into every consumer | `Examples/Common/` |
| `BVHUnion`/`BVHSmoothUnion` factories (CSG-16) | block dropping the `IF` suffix (a class and function template can't share a name); expose less than the constructors | delete, or `makeBVHUnion` |
| Brute-force query methods on both point-cloud classes (PC-12) | eight public "testing only" methods, duplicated | two free functions over positions |
| `VertexNormalWeight::None` (MESH-16) | not a valid pseudonormal; can produce wrong signs | — |

### 2.7 The virtual CSG layer and the tape front-end

Nothing built into the library derives from `ImplicitFunction` any more except the `*IF` wrappers
themselves. The layer is a closed loop: transforms and CSG only accept `ImplicitFunction`s, and
shapes and mesh SDFs are no longer `ImplicitFunction`s. The only adapter is test-local
(`Tests/TestShapeIF.hpp`). So today a user cannot, for example, translate a `TorusSDF` or
difference two meshes, even on the host (CSG-6, CSG-20).

The layer's nodes keep children and parameters `protected` with no getters, kind tag or visitor, so
a tape compiler handed a `shared_ptr<ImplicitFunction<T>>` can only treat it as an opaque host
callback (CSG-5). Two ways forward:
- **(a) A new value-semantic expression builder** as the tape's front-end. Today's `Union`,
  `Translate`, ... factories are re-pointed to it; `ImplicitFunction` survives only as the
  host-only "opaque user leaf" and as a reference oracle for tape tests. **Recommended.**
- **(b) A virtual `lower(TapeBuilder&)` on `ImplicitFunction`**, which keeps the class hierarchy as
  the front-end.

Either way, a tape containing an opaque user leaf must be marked host-only.

CSG-20 raises a related judgement call that reverses a recorded decision. A host-only
`ShapeIF<T, S>` adapter (virtual `value()` forwarding to `S::signedDistance`) reuses the existing
vtable rather than adding a dispatch mechanism. It would restore host-side transforms and CSG over
built-in shapes and meshes now, for users, the integrations and the `CSGUnion` example, and it
would give the tape a reference oracle over the same primitives (decision D5).

### 2.8 Header boundaries

The device-facing value types share headers with host-only machinery, so every device TU, and the
future interpreter, pulls in all of it:
- `BVH.hpp` (~2000 lines) mixes `PackedBVH` with host builders, `<functional>`, `<memory>` and the
  SFC (BVH-14).
- `CSG.hpp` mixes the blends and BVH unions with the `shared_ptr` combinators; `CSGImplem` includes
  `Transform` just to reach `Complement` (CSG-17).
- `Vec.hpp` pulls in `<ostream>` for one friend `operator<<` (MEM-21).

Split these into `PackedBVH.hpp`/`BVHBuild.hpp`, `Blend.hpp`/`BVHUnion.hpp`/legacy CSG, and a
separate Vec I/O header.

### 2.9 Verification: an oracle layer and a GPU harness

The tape's main test should be generic: for every opcode and every composition, the interpreter
equals direct evaluation, element-wise over a point array, in both precisions. That needs two
things the suite doesn't have.

**Checked references.**
- Six shapes (`InfiniteCylinderSDF`, `CapsuleSDF`, `InfiniteConeSDF`, `ConeSDF`,
  `RoundedCylinderSDF`, `PerlinSDF`) appear only in `AllShapes`. Their two checks are a memcpy'd
  copy evaluating equal to itself, which holds for any formula, and a device comparison that never
  runs (QA-6). Spot checks found them correct; the gap is the missing test. The brute-force
  surface-sampling check the CSG reviewer used would cover every shape generically (CSG-18).
- Mesh signs are tested only on the tetrahedron and dodecahedron. Concave edges and vertices, where
  the angle-weighted pseudonormal decides the sign, are never exercised. The examples compare the
  BVH SDFs against `FlatMeshSDF`, which runs the same pseudonormal code, so that is not an
  independent check (QA-7). Add a concave fixture checked by ray parity or winding number.
- `TestTransform` and `TestCSG` are written against `shared_ptr<ImplicitFunction>` through the
  test-local adapter. Restating their expectations as API-neutral "point → expected value" tables
  would let the same expectations drive the tape later.

**A GPU harness that can fail.**
- Every `[gpu]` test SKIPs before doing anything, so the CI comment that it "still verifies the
  host-side setup" is false (QA-5). Build pools, mirrors and rebased views before the device check.
- Kernels run `<<<1,1>>>` and reduce to one scalar sum, so errors cancel. `meshSDFProbeSum` sums
  `|d|`, which hides exactly the likeliest device failure: a sign error. Launch and synchronize
  errors are discarded (QA-13). Compare per-point arrays, launch many threads, check errors.
- Make the HIP compile lane required. It compiles today and is a cheap gate for a missing
  `EBGEOMETRY_HOST_DEVICE`. Add the HIP + AVX combination (§2.4).
- Structure the interpreter so the identical code path runs on the host in CI, since no device
  executes there. Plan an executing lane (a self-hosted runner) separately.

**CI gating.** `CI-passed` depends on the other jobs through `needs` but has no `if: always()`.
When a dependency fails it is *skipped*, and GitHub treats a skipped required check as passing
(QA-22, not verified against this repository's branch protection). CI runs only on `pull_request`,
never on pushes to `dev`/`main`. The Doxygen pre-commit hook filters on `\.(H|cpp)$`, so editing a
`Source/*.hpp` never triggers it; codespell names a nonexistent `Exec/` directory; CI builds Sphinx
without `-W`; and the `literalinclude` ban is not checked in CI (QA-20).

---

## 3. What the tape needs from the codebase

A checklist distilled from every reviewer's tape notes. Each item should be true before tape design
starts.

- [ ] **Correct reference values.** All §1 blockers fixed, plus CSG-9/10/11 (FiniteRepetition,
      Elongate, Capsule), with regression tests. The tape's golden values come from these formulas.
- [ ] **Visit-once BVHs.** Union opcodes (min, two-smallest, sums) require each primitive to be
      visited exactly once (BVH-1).
- [ ] **Layouts, not self-locating descriptors**, for everything the tape will embed (§2.1).
- [ ] **An adopt-pool-resident `PackedBVH` constructor** (§2.2), so the tape can produce BVHs
      without a host round trip. The single build path was dropped in item 17 (D2 amended), and a
      single leaf-size meaning is not needed: the tape's builder chooses its leaf setting
      explicitly, like any other consumer (BVH-2).
- [ ] **A K policy**: one K per precision for tape BVHs, or a K-generic opcode, plus a device stack
      budget that accounts for nesting (a union of `TriMeshSDF`s runs a traversal inside a traversal).
- [ ] **One mesh primitive** with a `uint32` element id and ISA-independent K/W (§2.3).
- [ ] **The device toolchain contract** in place and enforced by CI (§2.4).
- [ ] **Per-shape bounding volumes** (CSG-7). Every shape gets HOST_DEVICE
      `computeBoundingVolume()`, infinite for Plane, InfiniteCylinder, InfiniteCone and Perlin.
      Heterogeneous BVH unions in the tape need them, and today only mesh SDFs and unions have one.
- [ ] **Distance-quality classes** documented: exact / conservative (1-Lipschitz) / not a distance.
      BVH pruning is legal only above "not a distance". This precondition on `BVHUnionIF` is
      currently unstated (CSG-7); Perlin and an oversize `FiniteRepetition` violate it.
- [ ] **Shape parameter conventions settled** (CSG-6): centres for `RoundedBoxSDF` and
      `RoundedCylinderSDF`, one axis convention (today Cylinder is along y, Torus has a z normal, and
      cones open along −z), and outer dimensions. The tape freezes each opcode's parameter layout.
- [ ] **Blends as data**: add `ExpMaxOp` (CSG-15); in the tape the blend kind becomes a runtime enum
      in the clause rather than a template parameter.
- [ ] **An oracle layer**: host tests for all twelve shapes, a concave mesh-sign fixture, and
      API-neutral expectation tables for transforms and CSG (§2.9).
- [ ] **A GPU harness that can fail**, array-based and error-checked, and a required HIP compile
      lane (§2.9).
- [ ] **A front-end decision** (§2.7).
- [ ] **A retirement list agreed** (§2.6), so the tape isn't designed around what's leaving.

The CSG reviewer's report contains a full operation → math → SDF-property → opcode table, the case
for two register kinds (Vec3 point registers produced by point-map clauses, scalar value
registers), and the observation that the tape can't be purely linear: BVH-union leaves, `Blur`
(27 taps) and `Mollify` (n³ taps) all need "evaluate sub-program k at point q". That material
belongs in the tape design document; it is summarised in Appendix A.

---

## 4. Decisions

Taken by the maintainer on 30 September 2026. "Recommended" is what this audit proposed.

| # | Decision | Recommended | Taken | Affects |
|---|----------|-------------|-------|---------|
| D1 | Where a pool object keeps its location | Layout/handle split | **Two steps.** `PoolLocation` takes over the location logic now (fixing the managed-memory crash and the duplication); the layout/handle split is the first step of the tape work | MEM-1/2/7/8/12 |
| D2 | Retire instead of port (§2.6) | All seven | **All seven**: `Octree::Node`, `TreeBVH` + `pack`/`packWith`, both `traverse()`s and their aliases, `SphereT`, `SignedDistanceFunction`, SYCL/OpenACC branches and dead macros, `Random`/`SimpleTimer` out of the umbrella. **Amended during item 17:** `TreeBVH`, its partitioners and `pack`/`packWith` stay, as the flexible host builder third-party codes rely on; `PackedBVH::traverse()` is to become a template, device-callable custom traversal | §2.6 |
| D3 | Mesh SDFs and `Meta` | Only `TriMeshSDF`, `uint32` face id | **Keep all three mesh SDFs**; replace `Meta` with a `uint32_t` face id and a user-side array. `MeshSDF` leaves store that face index and resolve the face through the mesh (fixes MESH-4, shrinks leaves) | MESH-4/6/7/17 |
| D4 | Tape front-end | Value-type expression builder | **Value-type expression builder** | CSG-5 |
| D5 | Host-only shape adapter now | Maintainer's call | **No adapter. Retire `ImplicitFunction`** and the virtual transform/CSG layer, replaced by a trait (`signedDistance`, later `computeBoundingVolume`) checked with `static_assert`. Removed **in the same change that introduces the builder and a host evaluator**, so there is no gap; the transform/CSG test expectations move to the builder | CSG-5/6/14/20 |
| D6 | Device math | Own header | **Own HOST_DEVICE math header**, `T m_X[3]` in `Vec3T` | MEM-5/19/20 |
| D7 | K and W of device-visible types | Fixed defaults | **Fixed defaults, ISA-tuned values opt-in**; documented prominently, with an `Examples/` program. **Amended (#155):** a host-only build may define `EBGEOMETRY_HOST_TUNED_DEFAULTS`, which makes the default triangle-group width the host-tuned one (the width is then a property of the build, not of a translation unit); K and the point-group width stay 4, since widening them measured no faster | MESH-9, BVH-5/18 |
| D8 | Bad input | Always-on checks; parsers throw | **Always-on checks; parsers throw** | MEM-4/7, MESH-3/13, PC-13 |
| D9 | Point-cloud hash grid | Keep | **Keep**: fix, unify with `PointCloudBVH`, move onto the Pool | PC-2/4/10/11 |
| D10 | Shape parameter conventions | Placement + one axis convention | **Placement everywhere, one axis convention** | CSG-6/8 |
| D11 | BVH union names | Delete factories, rename classes | **Delete the factories; rename to `BVHUnion`/`BVHSmoothUnion`** | CSG-16 |
| D12 | SIMD flags for consumers | `none` for consumers | **`none` for consumers, `avx` when top-level** | QA-2 |
| D13 | Integrations | Port six, mark the rest | **Port the six; Shapes integrations and `CSGUnion` become tape acceptance tests** | QA-1/18 |
| D14 | Planning documents | Fold and delete | **Fold into PORTING.md and delete** | QA-11 |
| D15 | Required GPU check | HIP required | **HIP required, CUDA advisory**; `CI-passed` fails (not skips) when a job fails | QA-5/22 |

---

## 5. Fix plan

Each item is one reviewable change with its own regression tests.

### Phase 0 — correctness and safety

Status as of 30 September 2026, on `claude/pre-tape-audit`:

| Item | Status | Commit |
|------|--------|--------|
| 1. BVH visit-once (BVH-1), coincident points (PC-1) | Done | `b0ad100` |
| 2. Degenerate faces (MESH-1), fan triangulation (MESH-13, QA-8) | Done | `218d7fe` |
| 3. Parsers (MESH-2/3/20, QA-9) | Done | `b4481f3` |
| 4. Transform and shape formulas (CSG-1/2/3/4/8/9/10/11/15, QA-6) | Done | `c92d13d` |
| 5. Point clouds (PC-2/9/13) | Done; the `uint32` index type (PC-7) moves to the `Hit` unification in Phase 2 | `581a47b` |
| 6. Device build safety (MESH-5, PC-3, MEM-3/4/6) | Done | `fab7299` |
| 7. Small fixes (PC-15, MEM-10/13, MESH-10) | Done | `262d211` |
| 8. Oracle tests (QA-7) | Done; the shape property test landed with item 4 | `e18a785` |
| 9. Docs corrections (QA-10/12/23, MEM-9, BVH-9, MESH-18, PC-19) | Done | `4b68cfc`, `7aefdcd`, `b566783` |
| 10. CI and hook hygiene (QA-20/22, D15) | Done; GPU-CUDA stays advisory | `926969c` |

Corrections found while fixing:

- **MESH-1.** A zero normal for a zero-area face is not enough: the filler lies on a crease, and
  without it the crease's pseudonormals see only one side (765 wrong signs remain on the test mesh).
  The readers now repair T-junctions (`Soup::removeDegeneratePolygons`); the zero normal remains as
  the fallback for meshes built by hand.
- **CSG-9.** Fractional repetition counts already behaved as rounded (2.5 gives tile 3 in both the old
  and new code). What remained was `std::clamp`'s undefined behaviour for negative counts, its
  host-only assertion, and the missing documentation.
- **MESH-3.** Running every reader over corrupted copies of the fixtures under ASan found crashes in
  all four formats, beyond the ones listed, and a precision bug: binary PLY indices were converted
  through `T`, so with `T = float` any index above 2^24 rounded to a neighbouring vertex.
- **QA-7.** On the L-shaped fixture, a concave edge is 90 degrees, and there either wall's normal
  alone still gives the right sign, so that fixture cannot catch a broken edge pseudonormal. A second
  fixture with a narrow notch does. Neither fixture has a vertex that is the only closest feature to
  some point, so vertex-normal weighting stays covered by the existing BVH tests only.
- **QA-20.** Sphinx in CI now runs with `-W`. Locally the only warnings are the eight figures that
  CI renders before building.

Phase 0 is complete. Items 7–10 were:

7. **Small fixes**: `Random` argument evaluation order (PC-15); replace Vec3T's non-ordering
   `operator<` family with named predicates (MEM-10); `MeshT::deepCopy` of an empty mesh (MEM-13);
   the loaders' transient DCEL goes into a scratch pool rather than the caller's (MESH-10). `SphereT`
   is deleted with the other retirements (D2) rather than fixed.
8. **Oracle tests** (§2.9, QA-7): a concave mesh fixture with an independent sign check.
9. **Docs corrections** (QA-10/12/23, MEM-9, BVH-9, MESH-18, PC-19): stale composition claims in
   README/index/ImplemBVH/Implementation/mainpage; broken example-README links; outdated snippets;
   the stale `MeshSDF` note in CLAUDE.md.
10. **CI and hook hygiene** (QA-20/22, D15): `CI-passed` fails when a job fails, and the HIP job
    becomes one of its dependencies; CI on push to `dev`/`main`; Doxygen hook on `.hpp`; codespell
    paths; `CheckDocs.py` in CI; correct the `[gpu]` CI comments.

### Phase 1 — foundations

| Item | Status | Commit |
|------|--------|--------|
| 11. Error policy (D8) | Done | `df9ebdc`, `9505d11`, `f619cae` |
| 12. Device math header and toolchain contract (D6) | Done; the CUDA lane (advisory) now builds without `--expt-relaxed-constexpr`, and has not run yet | `c975031` |
| 13. CMake target (D12, QA-4/21) | Done; also install rules and `find_package` support, checked by a new CI job | `eb86481` |
| 14. Fixed K and W (D7) | Done; defaults are 4, host-tuned values opt-in, `Examples/HostTuning` | `92aae41` |
| 15. Location, step one (D1, MEM-2, MEM-8) | Done; the layout/handle split remains the first tape step | `3145b5a` |
| 16. GPU test harness (QA-5/13) | Done; the tests also run in every host build, emulated. No lane runs a real kernel yet (no GPU runner) | `2e4f700` |
| 17. Stack, empty leaves, SIMD rows, builder fixes (BVH-5/8/11/12/13) | Done, narrowly: a first, wider version (`6c58ae0`: one builder, a wide-node layout, `TreeBVH` retired) was reverted in `9481022`, since code outside EBGeometry relies on `TreeBVH`'s custom partitioners. BVH-2 is reframed as leaf-size settings per construction method (section 6); the template custom traversal is 17d | `051f53c` |
| 17c. Preset construction methods (BVH-2, in part) | Done: the enum, renamed `BVH::Build` → `BVH::Construction`, names each algorithm (`CentroidSplit`, `MidpointSplit`, `SAH`, `ClusterSAH`, `Morton`, `Nested`, `Hilbert`; `TopDown` removed), every library user accepts every value, and an unknown value aborts (MeshSDF/TriMeshSDF printed to `std::cerr` and packed an unpartitioned tree). `SphereT::define` aborts on an unknown algorithm or an empty list too. Leaf-size rules still differ per user | `8df456b` |
| 17d. Template custom traversal (D2 amended) | Done: `PackedBVH::traverse()` takes its four callbacks as template parameters and a fixed stack sized from the tree depth, so device code can call it; host code passing the `std::function` aliases still compiles. Children reach the orderer as `BVH::NodeAndKey` instead of `std::pair`. An empty BVH visits nothing | `64a205e` |
| 18. Header splits (§2.8, BVH-14, CSG-17, MEM-21) | Done: `EBGeometry_PackedBVH.hpp` (no SFC, no `TreeBVH`), `EBGeometry_TreeBVH.hpp` (`TreeBVH`, the partitioners, `pack()`/`packWith()`) and `EBGeometry_BVHBuild.hpp` (`PackedBVH`'s builder constructors); `EBGeometry_Blend.hpp` and `EBGeometry_BVHUnion.hpp` (neither pulls in `ImplicitFunction` or the transforms) beside the legacy `EBGeometry_CSG.hpp`; `operator<<` for `Vec3T`/`AABBT`/`SphereT` in `EBGeometry_StreamOperators.hpp`. `EBGeometry_BVH.hpp` and `EBGeometry_CSG.hpp` include the new headers, so existing includes still work. Other headers still include `<iostream>` for `std::cerr` | `9d4de69`, `68e975e`, `0773239` |
| 18b. Loading defective meshes (item 11 finding) | Done: `Parser::OnDefect::Warn` loads a mesh with inconsistent orientation, an over-shared edge or a fold, with a warning; `Throw` stays the default, and a face that visits a vertex twice always throws. The three rejected test models load, MeshSDF and TriMeshSDF agreeing. Also fixed: TestParser's scratch files raced between the parallel float and double runs | `cb25109`, `4f9afa3` |

Findings from item 11:

- **The "tree too deep" death test never reached the depth check.** Its K = 256 node needs 2048-byte
  alignment, above the Pool's 256; a Debug-only check on the alignment fired first, and Release would
  have misaligned the block silently. `PODVector` now rejects such a type at compile time (MEM-16),
  and the test builds a 101-level chain by hand.
- **Fuzzing.** 40 byte flips per fixture (598 inputs) found seven more ways a corrupted file reached a
  Debug abort, or undefined behaviour in Release: two STL facets merged by a damaged `endfacet`, faces
  that could not be joined into a half-edge mesh, and faces folded back onto each other. The readers
  now reject all three with a `ParseError`; the run ends with a `ParseError` or a mesh every time.
- **Real meshes now rejected.** Three of the submodule's 24 OBJ models throw: `beetle.obj` and
  `xyzrgb_dragon.obj` (inconsistent orientation or non-manifold edges) and `ogre.obj` (a fold). They
  already aborted in Debug; Release loaded them with undefined signs near the defect. Holes are still
  accepted. They now load with `Parser::OnDefect::Warn` (item 18b).

Findings from item 17:

- **Kept narrow on purpose.** Third-party codes use `TreeBVH` with their own partitioners to
  partition other object types, then pack into `PackedBVH`. Item 17 therefore changes internals
  only: the traversal stack, how empty leaves are caught, which nodes get SIMD child-box rows, and
  the bug fixes below. Every class keeps its API. D2 is amended accordingly.
- **Size.** On the armadillo mesh (100k faces) the MeshSDF BVH takes 19.6 MB instead of 31.1 MB and
  the TriMeshSDF BVH 33.0 MB instead of 35.5 MB, from dropping the SIMD row every leaf used to get.
  Distances are identical and query times unchanged.
- **Bugs found and fixed.** The binned SAH partitioner and ClusterSAH indexed their bins out of bounds
  when a range of centroids had a subnormal extent (the old code segfaults on 60 points at 2^-i);
  `bottomUpSortAndPartition` built one level too few at N = 3^5, 10^3 and other exact powers;
  `TreeBVH::traverse` read its stack entry after popping it. All three have tests that fail
  without the fix.
- **Deep trees on adversarial input.** SAH and midpoint splits give trees whose depth grows linearly
  on a geometric sequence of points (352 levels for 1100 points). The host limit is 256 levels, so
  such a build aborts with a message rather than overflowing the stack; an equal-count fallback
  would keep them shallow. It is independent of leaf size, but touches the same partitioners, so
  it fits the BVH-2 change.
- **Four integrations do not compile, independently of this item.** `Integrations/AMReX` and
  `Integrations/Chombo` `PackedSpheres`/`RandomCity` still call the BVH unions in a form older
  than the Pool; they are not built by CI. Item 22 (D13) ports them (and found two more).

11. **Error policy** (D8): an always-on `EBGEOMETRY_REQUIRE` for user input and one-time host checks;
    parsers throw a `ParseError` with file, line and reason.
12. **Device math header and toolchain contract** (D6, MEM-5): `EBGeometry_Math.hpp`, `T m_X[3]` in
    `Vec3T`, a lint against `std::min`/`std::max`/`std::numeric_limits` in device paths, written into
    PORTING.md.
13. **CMake target** (D12, QA-4/21): `none` SIMD default for consumers, `cxx_std_17` on the target,
    the GPU build documented in Sphinx.
14. **Fixed K and W** (D7): device-visible defaults fixed per precision, ISA-tuned values opt-in,
    documented, with an `Examples/` program.
15. **Location, step one** (D1): `PoolLocation` owns `base()`, attach, lineage checks and rebasing;
    location decided by value, so views onto managed/mapped memory work on the host (MEM-2), with
    tests using a fake host+device-accessible resource.
16. **GPU test harness** (§2.9, QA-5/13): array-based, many-thread, error-checked device tests.
17. **Single BVH builder, build spec, wide-node layout, device stack policy** (§2.2,
    BVH-2/3/4/5/8/11/12/13). A prerequisite for retiring `TreeBVH` (D2).
18. **Header splits** (§2.8).

### Phase 2 — consolidation and retirement

| Item | Status | Commit |
|------|--------|--------|
| 19a. Face ids for the mesh side (D3, MESH-4/6/7/17) | Done: no `Meta` template parameter on the DCEL classes, `Triangle`, `TriangleAoSoA`, the mesh SDFs or the parsers; a face's id is its index in the mesh (the readers keep file order, minus removed zero-area faces), and triangles carry the id of the face they were cut from. `MeshSDF`'s BVH stores face ids and reads the faces from the mesh, so a flip or a reconcile-and-refit after the build is seen (MESH-4); its primitive array is 4 bytes per face instead of 88 (MESH-17), and queries on the armadillo mesh are about 15% slower from the indirection. `getClosestFace()`, on all three mesh SDFs and device-callable, returns the closest face's id and signed distance, replacing `MeshSDF::getClosestFaces` and `TriMeshSDF::getClosestTriangle` (MESH-7). The point clouds keep `Meta` until item 20 | `f7f07ce` |
| 19b. Parser renames (MESH-12), one polygon-soup container (MESH-11) | Done: `PolygonSoup<T>` replaces `STL`, `PLY`, `OBJ` and `VTK`. `readSTL`/`readPLY`/`readOBJ`/`readVTK` and the new `readIntoPolygonSoup` return it already cleaned: duplicate vertices merged, degenerate faces removed, T-junctions repaired, so face i and vertex i of the soup are face i and vertex i of every mesh built from the file. PLY and VTK scalar properties are kept per vertex and per face and remapped by the cleaning (`Soup::compress` and `removeDegeneratePolygons` now return their index maps), so a face property is indexed by the face id `getClosestFace()` reports; a VTK array that cannot be aligned is dropped with a warning. `PolygonSoup::convertToDCEL` returns the mesh by value, and `convertToTriangles` feeds `readIntoTriangles`; `Soup::readSoupIntoDCEL` and `Parser::requireValidSoup` are gone. The readers are named after what they return: `readIntoMesh` → `readIntoFlatMeshSDF`, `readIntoPackedBVH` → `readIntoMeshSDF`, `readIntoTriangleBVH` → `readIntoTriMeshSDF`, with no aliases. Every reader takes file, pool, construction, then a `BVH::ConstructionOptions` overload, then the defect policy; `readIntoMeshSDF`'s K defaults to `BVH::DefaultBranchingRatio<T>()`, and `readIntoTriMeshSDF` drops `maxLeafGroups` for `TriMeshSDF::defaultConstructionOptions(4)` or explicit options. | |
| #155. Host-tuned triangle width as a build switch (D7 amended) | Done: `EBGEOMETRY_HOST_TUNED_DEFAULTS` makes `TriangleSoA::DefaultWidth` return `TriangleSoA::HostWidth`; the headers reject it in CUDA/HIP translation units and CMake rejects it with a GPU backend; the CMake option is on for a top-level, non-GPU build. Measured about 18% faster `float` `TriMeshSDF` queries on AVX2; widening K or the point groups was no faster, and K = 16 roughly doubled query times under AVX-512, so they stay 4 | |
| BVH-2. Leaf-size settings per construction method | Done: `BVH::ConstructionOptions` holds one setting per family of methods (`maxLeafSize` for the top-down splits, `targetLeafSize` for the space-filling curves, `cluster` for ClusterSAH), and a method reads only its own. `MeshSDF`, `TriMeshSDF`, `BVHUnionIF` and `BVHSmoothUnionIF` gain a constructor taking it, and a static `defaultConstructionOptions()` that reproduces their trees without it (checked node for node against `dev` on the cow mesh and on 3000 spheres, for all seven methods). `TreeBVH::bottomUpSortAndPartition` takes the target (default 1, the old tree). One default changes: `TriMeshSDF`'s space-filling curves now honour `maxLeafGroups` as a target of `maxLeafGroups·W` instead of ignoring it, giving 4× fewer nodes and, on the armadillo mesh, 20–34% faster Morton and Hilbert queries; Nested, already ~50× slower than SAH, got 17–52% slower. The `readInto*` functions get the setting in 19b, with their options argument. The equal-count fallback for deep trees (item 17 finding) is not part of it | |
| 20. Point clouds (D9, PC-4/7/8/10/12/14) | Done: `PointCloudBVH` and `PointCloudHashGrid` return one `PointCloud::Hit<T>` (a `uint32` cloud index, `PointCloud::InvalidIndex` for a miss) and share one host-device `PointCloud::KBest` set (PC-4, PC-7). `PointAoSoA`, `PointCloudBVH` and `PointCloudHashGrid` lose `Meta`: `PointAoSoA<T, W>` carries `uint32` point ids like `TriangleAoSoA`, and per-point user data lives in the caller's array. Leaf scans stop at `numValid()` instead of de-duplicating every insert (PC-8). The grid is built in a `Pool`, trivially copyable, with `rebasedView()`/`deepCopy()` and device-callable queries (D9), and its stopping rule uses the true cell faces less a few ulps instead of a one-cell slack (PC-10). The eight brute-force member methods are two free functions, `PointCloud::closestPoint(s)BruteForce()`, callable on a device (PC-12). On 200k points: k = 8 BVH queries 20–25% faster, grid queries 2.5–4× faster and level with the BVH on a uniform cloud; on a clustered cloud the grid is 2–3× faster but still far behind the BVH (PC-11 stays open). PC-14: `PointAoSoA`/`TriangleAoSoA` now carry ids the same way; the two `DefaultWidth` helpers stay separate, since #155 made them differ | |
| 21. Retirements (D2) and union names (D11) | Done: `Octree::Node` is gone. `approximateBoundingVolumeOctree` walks the subdivision depth-first with a fixed stack of integer-addressed cells and a running min/max, allocates nothing, is device-callable when the function is, returns an `AABBT` (no `BV` parameter) and rejects a depth above `MaxOctreeDepth` (24) (PC-5, PC-17, CSG-12, CSG-13). `SignedDistanceFunction` is gone; a free `normal(f, p, delta)` replaces its `normal()` (CSG-14). Both live in the new `EBGeometry_FunctionQueries.hpp`. `SphereT` is gone (BVH-6). `VertexNormalWeight` is gone with its `None` value; `MeshT::reconcile()` always computes the angle-weighted pseudonormal (MESH-16). The SYCL/OpenACC branches and `EBGEOMETRY_DEVICE`/`INLINE`/`ROUTINE` are gone; `EBGEOMETRY_GLOBAL` stays, used by the device-test harness (MEM-11). No `#error` for other compilers: a SYCL or OpenACC program may still include EBGeometry for host use. `Random` and `SimpleTimer` stay in `Source/` but leave the umbrella header; the examples include them directly, which keeps each example folder buildable on its own (PC-16). The union factory functions are deleted and `BVHUnionIF`/`BVHSmoothUnionIF` are renamed `BVHUnion`/`BVHSmoothUnion` (CSG-16). `TreeBVH`, its partitioners, `pack`/`packWith` and the `std::function` aliases stay (D2 amended). | |
| 22. Integrations and examples (D13, QA-1/17/18/19) | Done: six integrations ported, not four as this audit counted: AMReX `PaintEB` and Chombo `MeshSDF` did not compile either (the Chombo one also read meshes from a `Resources/` folder that no longer exists). The AMReX `MeshSDF`, `PaintEB`, `PackedSpheres` and `RandomCity` implicit functions are `amrex::GPUable` and hold a host and a device descriptor; the Chombo `MeshSDF`, `PackedSpheres` and `RandomCity` hold their geometry by value and the pool by `shared_ptr`. `bvh = false` now visits every primitive of the same union instead of building a second, virtual one. All seven were compiled and run against AMReX's development branch and Chombo 3.2 on CPUs (with and without the BVH giving identical output), and the AMReX ones compiled for HIP; CI still does not build them (QA-1). The two `Shapes` integrations and the `CSGUnion` example are tape acceptance tests (`Tests/TapeAcceptance/`, not built), and `CSGUnion` leaves `Examples/` and CI (QA-18). Every example checks its results point by point against a brute-force scan or hand-worked distances, with fixed seeds, and exits nonzero on a mismatch; MeshSDF samples around the mesh rather than around the origin (QA-17). The examples are the folders: `Examples/CMakeLists.txt` and the CI lanes glob `Examples/*/main.cpp`, so CI now runs all eleven, not five to seven; each compiler lane is one job looping over them; the GNU and CMake lanes build with `-Werror` (the GNU lane's `-Wsign-conversion` stays a warning: the library has about 250 of them, left for a separate cleanup; its one `-Wshadow` and one `-Wswitch-default` are fixed); the CMake example lanes no longer build the unit tests; and the examples no longer force `-O3 -march=native`, following the project's build type and `EBGEOMETRY_SIMD` (standalone they default to `Release`, and `HostTuning` keeps `-march=native`) (QA-19) | |
| 23. Planning documents (D14, QA-11) | Done: `PLAN.md` and `BVH_PORT_SUMMARY.md` are folded into `PORTING.md` and deleted. `PORTING.md` now gives the current status (every device class ported, Phases 0–2 done), the order the work was done in with its PR numbers, what remains before the tape (item 24, a toolkit that can run kernels), the tape (item 25) and what follows it (batched traversal, device builds, the DCEL reconcile chain, more backends), and a "Decisions and their evidence" section carrying what the two documents still had to say: why value types and not virtual interfaces, the single traversal loop and the alignment bug its exact-equality sweep found, why `PackedBVH<T, uint32_t, K>` replaced the storage policies (the torus measurements), why copies are shallow, the leaf-size measurements and what came of them, and the device stack budget. Its stale claims are corrected: the status line, "this branch" for the BVH port, the GPU jobs described as not required (HIP has been since D15), and the CSG-union guard as an acceptance test. `REUSE.toml` drops the two deleted files and `TODO.md`, which never existed | |

19. **Mesh SDFs** (D3): `uint32` face id replaces `Meta`; `MeshSDF` leaves store face indices;
    `getClosestFace` on `pruneTraverse`; parser renames (MESH-12); one polygon-soup container (MESH-11).
20. **Point clouds** (D9): one `Hit` type with a `uint32` index, a shared k-best helper, the hash grid
    on Pool/`PODVector` (PC-4/7/8/10/12/14).
21. **Retirements** (D2) and **union names** (D11). `TreeBVH`, `pack`/`packWith` and the
    partitioners stay (D2 amended); `PackedBVH::traverse()` became the template custom traversal
    (17d) instead of being retired.
22. **Integrations and examples** (D13, QA-1/17/18/19). Follow-up: the library's roughly 250
    `-Wsign-conversion` warnings; once they are fixed, drop `-Wno-error=sign-conversion` from the
    `Linux-GNU` lane.
23. **Planning documents** (D14).

### Phase 3 — shape API, then the tape

24. **Shape conventions** (D10) and per-shape `computeBoundingVolume()` (CSG-7); distance-quality
    classes documented.
25. **Tape, first steps** (D1 step two, D4, D5): the layout/handle split; the value-type builder with a
    host evaluator; `ImplicitFunction` and the virtual transform/CSG layer retired in the same change,
    with their test expectations carried over; a shape trait checked with `static_assert`.
26. Close the checkpoint in PORTING.md and move the tape design inputs (Appendix A) into the tape
    design document, and item 27 below into PORTING.md. This file is then deleted.

### After the tape — further device backends

27. **More device backends**, decided while item 21 removed the fictional SYCL and OpenACC
    branches. Each is a real backend or none: a memory resource, device-pass detection, the GPU test
    harness running on it, and a CI lane, as CUDA and HIP have now. Deferred until the tape exists,
    since its interpreter becomes the main device code; the tape keeps to the subset that makes them
    possible (Appendix A, "Backend-neutral device code").
    - **SYCL, first.** AMReX's third backend (Intel GPUs). Device code needs no decoration, so the
      macros stay empty. Work: a `sycl::queue`-backed memory resource (`malloc_device`/
      `malloc_shared`, `queue.memcpy` for `Pool::mirror`); device-pass detection through
      `__SYCL_DEVICE_ONLY__` for the device assertion path; the test harness launching through
      `parallel_for`; a CI lane with oneAPI DPC++, whose CPU device runs the kernels without a GPU.
    - **OpenMP offload, if a user needs it.** `begin/end declare target` around the device-callable
      code (OpenMP 5.x also marks functions used in target regions implicitly); a memory resource on
      `omp_target_alloc`/`omp_target_memcpy`; device-pass detection, which OpenMP does not
      standardise (compiler macros or `declare variant`); the harness as `target teams distribute
      parallel for`; a CI lane with an offload-capable compiler. No current user (AMReX uses CUDA,
      HIP and SYCL), and compiler support is uneven.
    - **OpenACC, declined.** Every device function and template instantiation needs an `acc routine`
      directive, C++ templates and classes are where compiler support is weakest (nvc++ is the only
      solid one), and its users are mostly Fortran and C codes. An nvc++ OpenACC application can
      already call the CUDA path.

The MINOR and NIT items not named above are folded into whichever PR touches the same file; they
are listed per area in section 6.

---

## 6. Findings by area

"Verified" means yes (read or reproduced by the reviewer), **R** (also re-run independently for this
report), or no (judgement).

### MEM — memory model, GPU macros, Vec

| ID | Sev | Cat | Finding | Where | Verified |
|----|-----|-----|---------|-------|----------|
| MEM-1 | MAJOR | design/tape | Descriptors locate themselves; location logic duplicated in `MeshT`/`PackedBVH` and forwarded by six classes; `IsPoolResident` is duck-typed | DCEL_MeshImplem 37-117, BVHImplem 427-525, CSG 421-464 | yes |
| MEM-2 | MAJOR | correctness/gpu | Views onto managed/mapped pools crash on host; building directly in managed memory impossible | DCEL_MeshImplem 79-101, BVHImplem 480-506 | yes |
| MEM-3 | MAJOR | correctness/gpu | ODR: `Pool::mirror` differs between host and CUDA/HIP TUs | PoolImplem 184-206 | yes |
| MEM-4 | MAJOR | robustness | Host allocation failure only EXPECT-checked; Release continues with a null base | MemoryResourceImplem 40-44 | yes |
| MEM-5 | MAJOR | gpu/api | `--expt-relaxed-constexpr` required under nvcc but only set on test targets | Tests/CMakeLists 107, Vec 613 | no |
| MEM-6 | MINOR | gpu | Device `EBGEOMETRY_EXPECT` disappears under `NDEBUG` | Macros 52-57 | yes |
| MEM-7 | MINOR | robustness | `freeze`/`rebasedView` preconditions Debug-only; frozen pool silently grows in Release | PoolImplem 111-114 | yes |
| MEM-8 | MINOR | api | Mirror-lineage checks only work from the root pool | Pool 232-246 | yes |
| MEM-9 | MINOR | docs | Headers still describe the "build phase / query phase" model; MemoryModel.rst says two classes use it (eight do) | Pool 8-19, PODVector 18-45 | yes |
| MEM-10 | MINOR | api | Vec3T `operator<` is not a strict weak ordering; `std::set` of 4 distinct points has size 1 | Vec 388-433 | yes |
| MEM-11 | MINOR | gpu/hygiene | SYCL/OpenACC support fictional; four macros dead | GPU 26-115 | yes |
| MEM-12 | MINOR | api | `base()` const-correctness inconsistent; four `const_cast`s | DCEL_MeshImplem 39, BVHImplem 443… | yes |
| MEM-13 | MINOR | api | `MeshT::deepCopy` of an empty mesh aborts | DCEL_MeshImplem 128 | yes |
| MEM-14 | MINOR | tests | No tests for managed-pool host views, intermediate-mirror rebasing, Vec3T clamp/`<` | Tests | yes |
| MEM-15 | NIT | robustness | `Pool::reserve` size overflow wraps silently | PoolImplem 116-118 | yes |
| MEM-16 | NIT | robustness | No `static_assert(alignof(T) <= PoolBaseAlign)` in `PODVector` | PODVector 117 | — |
| MEM-17 | NIT | api | Vec3T scalar-left operators unconstrained; `2.0 * Vec2T<float>` doesn't compile; dead free `operator*` | Vec 623-725 | yes |
| MEM-18 | NIT | design | Vec2T and Vec3T have different API styles; constexpr `length()` calls `std::sqrt` | Vec 36-614 | — |
| MEM-19 | NIT | gpu/perf | Vec3T is not trivially default-constructible (blocks `__shared__` arrays) | VecImplem 210-213 | — |
| MEM-20 | NIT | numerics | `v/s` is a reciprocal multiply; min/max propagate NaN asymmetrically | VecImplem 339-345 | — |
| MEM-21 | NIT | hygiene | Unused `<algorithm>`, `<ostream>` in Vec.hpp; no include guard in EBGeometry.hpp; `std::aligned_alloc` on MSVC | Vec 16-20 | — |
| MEM-22 | NIT | robustness | Static-lifetime device pools may abort at exit (`cudaErrorCudartUnloading`) | MemoryResourceImplem 80-89 | no |

### BVH — hierarchies, partitioners, SFC, bounding volumes

| ID | Sev | Cat | Finding | Where | Verified |
|----|-----|-----|---------|-------|----------|
| BVH-1 | BLOCKER | correctness | SFC constructor pads leaves by duplicating the last one: wrong smooth unions under Morton/Nested, >N primitive evaluations near that leaf, ~2× nodes | BVHImplem 814-844 | **R** |
| BVH-2 | MINOR | api | `Build` and leaf size mean different things in each consumer; ClusterSAH/Midpoint/Hilbert unreachable. *Reassessed:* the enum half was fixed in 17c (`BVH::Construction`, every value reachable everywhere). The leaf-size half is a tuning gap, not a correctness problem: `MeshSDF` and `BVHUnionIF` expose no leaf size, `TriMeshSDF`'s `maxLeafGroups` is ignored by the space-filling-curve methods, and the defaults are not documented per method. A universal "at most n per leaf" was rejected: the methods have different natural settings (a maximum for top-down splits, an even-split target for the curves, a cluster size for ClusterSAH), so leaf size is set per method. Not a tape prerequisite | MeshDistanceFunctionsImplem 55-265, CSGImplem 103-118 | yes |
| BVH-3 | MAJOR | design | `TreeBVH` no longer justified; build logic duplicated ~7 ways | BVH 808-1134, BVHImplem | yes |
| BVH-4 | MAJOR | design | `std::function`/`shared_ptr` partitioners and a `TreeBVH&` leaf predicate couple `PackedBVH` to `TreeBVH` | BVH 221-234, BVHImplem 914-940 | yes |
| BVH-5 | MAJOR | gpu | 64-entry device stack: depth ≤ 5 at K=16, ≤ 10 at K=8; realistic trees abort in `rebasedView` | BVH 1912-1957 | yes |
| BVH-6 | MAJOR | correctness | `SphereT(vector<SphereT>)` fails to enclose its inputs in 49% of random sets | BoundingVolumesImplem 44-64 | **R** |
| BVH-7 | MAJOR | design | Four traversal loops; `TreeBVH::traverse` reads a destroyed `shared_ptr` and terminates on stack roots | BVHImplem 341-379, 1263-1307 | yes |
| BVH-8 | MAJOR | tape | Node boxes stored twice; zeroed SoA rows for leaves; a zero-primitive leaf reads as interior | BVH 1188-1209, BVHImplem 583-624 | yes |
| BVH-9 | MINOR | docs | Stale docs: copy constructor called a deep copy, missing AVX-512, `pruneTraverse` fallback, padding "harmless" | BVH.hpp various, ImplemBVH.rst 154-160, 572-578 | yes |
| BVH-10 | MINOR | api | `pack()` returns `shared_ptr`; `appendAliased`; constructor overloads selected by tag types | BVH 1066-1086, 132-183 | yes |
| BVH-11 | MINOR | performance | No pairwise AABB union; every node union allocates a vector; refit host-only | BoundingVolumes | yes |
| BVH-12 | MINOR | robustness | Anisotropic SFC normalisation; `floor(log N/log K)` off at exact powers, UB at N=0; denormal extent → NaN→int | SFCImplem 248, BVHImplem 246 | yes |
| BVH-13 | MINOR | robustness | Empty-BVH accessors read node 0; unchecked size casts in `finalize` | BVHImplem 1247-1261 | no |
| BVH-14 | MINOR | hygiene | Device-facing `PackedBVH` shares a header with host-only builders | BVH 14-37 | yes |
| BVH-15 | MINOR | tests | No smooth-union test under Morton/Nested, no visit-once test, no `SphereT` enclosure test, no K=8/16 depth test | Tests | yes |
| BVH-16 | NIT | hygiene | `noexcept` on allocating functions | various | yes |
| BVH-17 | NIT | correctness | SIMD/scalar "bit-identical" only for finite inputs (NaN handling differs) | BVHImplem 1644-1658 | no |
| BVH-18 | NIT | gpu | ISA-dependent types and inline bodies across TUs (ODR) | BVH | yes |

### MESH — DCEL, mesh SDFs, triangles, parsers

| ID | Sev | Cat | Finding | Where | Verified |
|----|-----|-----|---------|-------|----------|
| MESH-1 | BLOCKER | correctness | Zero-area face → NaN normal → wrong signs over a large region | DCEL_FaceImplem 88-154, DCEL_VertexImplem 245-263 | **R** |
| MESH-2 | BLOCKER | correctness | Binary STL split by attribute bytes; only the first group returned | ParserImplem 321-341 | **R** |
| MESH-3 | MAJOR | robustness | Parser crashes/UB on empty, truncated, missing, or malformed files | ParserImplem 190-341, 1750-1805 | yes |
| MESH-4 | MAJOR | correctness/design | `MeshSDF`/`TriMeshSDF` pack copies of derived geometry; flip after build gives mixed signs | MeshDistanceFunctionsImplem 67-171 | yes |
| MESH-5 | MAJOR | gpu | `TriangleSoAT` SIMD blocks not excluded from device compilation | TriangleSoAImplem 101, 428, 628, 782 | yes |
| MESH-6 | MAJOR | design/tape | `Meta` carries no data but multiplies types | DCEL.hpp 28, parsers, MeshDistanceFunctions 441-451 | yes |
| MESH-7 | MAJOR | api | `getClosestFaces` returns packed-order indices and traversal history; host-only | MeshDistanceFunctionsImplem 403-467 | yes |
| MESH-8 | MAJOR | design | Three mesh SDFs and eight triangle kernels; narrow to one | MeshDistanceFunctions, Triangle*, TriangleSoA* | no |
| MESH-9 | MAJOR | gpu/tape | ISA-dependent K/W defaults make the SDF a different type in host and device TUs | Parser.hpp, TriangleSoA.hpp | partial |
| MESH-10 | MAJOR | perf/gpu | Loaders leave the transient DCEL in the caller's pool; it is mirrored to the device | ParserImplem 1846-1891 | yes |
| MESH-11 | MINOR | design | Four near-identical format container classes returning vestigial `shared_ptr<MeshT>` | STL/OBJ/PLY/VTK | yes |
| MESH-12 | MINOR | api | Parser names don't match the types they return; inconsistent defaults and argument order | Parser.hpp | yes |
| MESH-13 | MINOR | robustness | `TriMeshSDF` silently truncates non-triangle faces in Release | MeshDistanceFunctionsImplem 145-175 | yes |
| MESH-14 | MINOR | robustness | Unknown `Build` → silent single-leaf tree; switch duplicated three times | MeshDistanceFunctionsImplem 83-265 | yes |
| MESH-15 | MINOR | robustness | Topology assumptions Debug-only; `EdgeIteratorT` can spin forever; inconsistent orientation accepted | DCEL_VertexImplem 215-247, SoupImplem 249-261 | no |
| MESH-16 | MINOR | design | `VertexNormalWeight::None` is not a valid pseudonormal | DCEL.hpp 73-77 | no |
| MESH-17 | MINOR | performance | Packed `FaceT` (88 B) carries unused data and a per-face algorithm switch | DCEL_Face 388-433 | yes |
| MESH-18 | MINOR | docs | Stale comments (edge numbering, `std::vector` storage, `size()` "in bytes", ...) | various | yes |
| MESH-19 | NIT | correctness | Sign-of-zero convention differs between SIMD and scalar triangle paths | TriangleSoAImplem | no |
| MESH-20 | NIT | tests | No binary parser fixtures; no degenerate/flip/non-triangle tests | Tests/data | yes |

### CSG — implicit functions, CSG, transforms, shapes

| ID | Sev | Cat | Finding | Where | Verified |
|----|-----|-----|---------|-------|----------|
| CSG-1 | BLOCKER | correctness | `Scale` with s<0 inverts the shape | TransformImplem 249-269 | **R** |
| CSG-2 | MAJOR | correctness | `Mollify` kernel inside-out (negative centre weight) | TransformImplem 356-386 | **R** |
| CSG-3 | MAJOR | correctness | `RoundedBoxSDF` not an SDF inside | AnalyticDistanceFunctions 1305-1312 | **R** |
| CSG-4 | MAJOR | correctness | `ExpMinOp` overflows to ±inf (float: ~100·s) | CSG 199-221 | **R** |
| CSG-5 | MAJOR | tape | Virtual CSG/transform graph can't be looked into | Transform.hpp, CSG.hpp | yes |
| CSG-6 | MAJOR | design | Built-in shapes can't be placed or composed; inconsistent axis/size conventions | AnalyticDistanceFunctions | yes |
| CSG-7 | MAJOR | tape | No per-shape bounding volumes; BVH-union pruning precondition undocumented | AnalyticDistanceFunctions, CSG 470-491 | yes |
| CSG-8 | MINOR | design | `PerlinSDF` is not an SDF, can't be combined, default isn't zero, persistence 0 → NaN | AnalyticDistanceFunctions 1326-1613 | **R** |
| CSG-9 | MINOR | correctness | `FiniteRepetition`: non-integer counts, `std::clamp` UB and host-only, non-conservative for off-centre bases | CSGImplem 912-945 | **R** |
| CSG-10 | MINOR | correctness | `Elongate` not an SDF inside the core | TransformImplem 424-434 | **R** |
| CSG-11 | MINOR | robustness | Capsule with tip distance ≤ 2r gives NaN or wrong shape | AnalyticDistanceFunctions 913-976 | yes |
| CSG-12 | MINOR | correctness | `Octree::Node::traverse` visits children in reverse of its contract (also PC-17) | OctreeImplem 179-209 | yes |
| CSG-13 | MINOR | design | `approximateBoundingVolumeOctree` doesn't need an octree; dead code; ±max on failure | ImplicitFunctionImplem 121-259 | yes |
| CSG-14 | MINOR | design | `SignedDistanceFunction` has no implementers; stale comment; holds the only `normal()` | SignedDistanceFunction.hpp | yes |
| CSG-15 | MINOR | api | Two blend mechanisms (`std::function` vs template functor); no `ExpMaxOp` | CSG.hpp | yes |
| CSG-16 | MINOR | api | Dropping `IF` collides with the `BVHUnion` factory functions | CSG 492-794 | yes |
| CSG-17 | MINOR | hygiene | `CSG.hpp` mixes virtual host-only combinators with device value types | CSG 25-29 | yes |
| CSG-18 | MINOR | tests | Missing tests for each bug above; suggests a brute-force surface-sampling shape test | Tests | yes |
| CSG-19 | NIT | docs | Annular "thickness" off by 2×; mixed `const` in `shared_ptr` members; allocating `noexcept` ctors; `std::pow` per `BlurIF::value`; Blur docs | Transform.hpp, ImplemCSG.rst 185-188 | yes |
| CSG-20 | MINOR | design | Judgement: a host-only `ShapeIF` adapter would cost less than it saves (reverses a decision) | PORTING.md step 4 | no |

The CSG reviewer also confirmed as **correct**: Rotate on all three axes, Blur's weights, `SmoothMinOp`/
`SmoothMaxOp`, the pruning bounds in both BVH unions, multi-subtrahend Difference, and the Cone,
InfiniteCone, Cylinder, Capsule, RoundedCylinder and Torus formulas (by brute-force surface sampling).

### PC — point clouds and utilities

| ID | Sev | Cat | Finding | Where | Verified |
|----|-----|-----|---------|-------|----------|
| PC-1 | BLOCKER | correctness | `PointCloudBVH` aborts on clouds with coincident points | PointCloudBVHImplem 206-243 | **R** |
| PC-2 | MAJOR | correctness | Hash grid: float→int overflow for far queries (UB, wrong neighbour) | PointCloudHashGridImplem 161, 282-304 | **R** |
| PC-3 | MAJOR | gpu | `PointSoAT` SIMD paths not guarded for the device pass | PointSoAImplem 82, 123, 144 | yes |
| PC-4 | MAJOR | design | The two "interchangeable" point-cloud classes duplicate each other and have drifted apart | PointCloudHashGrid.hpp, PointCloudBVH.hpp | yes |
| PC-5 | MAJOR | design | Retire `Octree::Node` rather than port it | Octree.hpp | no |
| PC-6 | MAJOR | gpu | `PackedBVH` construction contract blocks a device-side build | BVH.hpp 1495, BVHImplem 1376-1420 | no |
| PC-7 | MINOR | performance | Cloud index stored as `size_t` though capped at uint32 | PointCloudBVH.hpp 83, 111 | yes |
| PC-8 | MINOR | design | Padded lanes handled by an O(k) de-dupe per insert instead of exposing the valid count | PointAoSoA 172, PointSoA 223 | yes |
| PC-9 | MINOR | api | Miss is `Hit{0, max}`, indistinguishable from point 0; `allNearestNeighbors` drops counts | PointCloudBVH.hpp 104-113 | yes |
| PC-10 | MINOR | performance | Hash grid's one-cell stopping slack forces a second shell; 2–2.5× slower than BVH on uniform clouds | PointCloudHashGridImplem 244-304 | yes |
| PC-11 | MINOR | performance | Hash-grid cell sizing assumes a volumetric cloud | PointCloudHashGridImplem 63-75 | no |
| PC-12 | MINOR | api | Brute-force reference methods are public API on both classes | PointCloudBVH.hpp 187-235 | yes |
| PC-13 | MINOR | robustness | Constructor input validation Debug-only | PointCloudBVHImplem 33-73 | yes |
| PC-14 | MINOR | design | `PointSoA`/`TriangleSoA` naming and width helpers inconsistent and duplicated | PointSoA.hpp, TriangleSoA.hpp | yes |
| PC-15 | MINOR | correctness | `Random::samplePoints` depends on argument evaluation order (gcc ≠ clang) | RandomImplem 40 | yes |
| PC-16 | MINOR | design | `Random` and `SimpleTimer` don't belong in the umbrella header | EBGeometry.hpp 38, 42 | yes |
| PC-17 | MINOR | docs | `Octree::traverse` child-order docs wrong (duplicate of CSG-12) | Octree.hpp 120-121 | yes |
| PC-18 | MINOR | robustness | `Octree::Node` `noexcept` + allocation; "no-op with diagnostic" is actually an abort | Octree.hpp 218-259 | yes |
| PC-19 | MINOR | docs | Docs say `PointCloudBVH` can be composed into a BVH/CSG; it can't | PointCloudHashGrid.hpp 46-47, ImplemPointCloud.rst 70-71 | yes |
| PC-20 | NIT | hygiene | Missing blank lines around a loop (CLAUDE.md convention) | PointSoAImplem 236-240 | yes |
| PC-21 | NIT | tests | Test gaps: coincident BVH points, far-field grid queries, k > N−1 padding, HIP+SIMD compile | Tests | yes |

### QA — tests, examples, integrations, build, CI, docs

| ID | Sev | Cat | Finding | Where | Verified |
|----|-----|-----|---------|-------|----------|
| QA-1 | MAJOR | correctness | 8 of 9 integrations don't compile against the current API; docs list all nine as available | Integrations/, Integrations.rst 20-44 | yes |
| QA-2 | MAJOR | design | Default `EBGEOMETRY_SIMD=avx` is forced onto every consumer of the CMake target | CMakeLists 45-67, Building.rst 22-82 | **R** |
| QA-3 | MAJOR | gpu | ISA-dependent default K/W with CXX-only SIMD flags: host and device can disagree on a type (duplicate of MESH-9) | BVH.hpp 93-113, CMakeLists 66 | partial |
| QA-4 | MAJOR | gpu | CUDA consumers get no required flags (MEM-5); Sphinx has no GPU build instructions | CMakeLists, Building.rst | yes |
| QA-5 | MAJOR | tests | No device code executes anywhere; GPU lanes non-blocking; CI comments overstate coverage | Tests/TestGPU.hpp, CI.yml 438-492 | **R** |
| QA-6 | MAJOR | tests | Six analytic shapes have no host correctness test | TestAnalyticSDF 160-233 | yes |
| QA-7 | MAJOR | tests | Mesh-SDF sign only tested on convex fixtures; no independent oracle | Tests/data, TestBVH, TestDCEL | yes |
| QA-8 | MAJOR | docs | Parsers docs promise triangulation; `readIntoTriangleBVH` keeps only the first three vertices (0.559 instead of 0.5 over a dropped quad half) | Parsers.rst 50-61 | yes |
| QA-9 | MAJOR | robustness | A missing mesh file segfaults `TriMeshSDF`/`MeshSDF` in Release; the `MeshSDF` example crashes without the submodule (see MESH-3) | MeshDistanceFunctionsImplem 567 | yes |
| QA-10 | MAJOR | docs | README/index "composable with transforms", ImplemBVH mesh-instancing text, CLAUDE.md `shared_ptr` note, "exactly two" SIMD places, mainpage link | README 29, index.rst 20, ImplemBVH 689-693, … | yes |
| QA-11 | MINOR | docs | PORTING/PLAN/BVH_PORT_SUMMARY stale and contradictory; `TODO.md` in REUSE.toml doesn't exist | PORTING.md, PLAN.md | yes |
| QA-12 | MINOR | docs | 37 broken links from example READMEs to nonexistent Sphinx pages | Examples/*/README.md | yes |
| QA-13 | MINOR | tests | GPU tests sign-blind (sums of absolute distances), `<<<1,1>>>`, launch/sync errors discarded | TestGPU.hpp, TestBVH 2861-2870 | yes |
| QA-14 | MINOR | tests | `InstantiateAll.cpp` misses `MeshSDF`, `TriMeshSDF`, unions over pool-resident primitives, `PackedBVH<T,uint32_t,K>`; non-host memory resources untested | InstantiateAll 22-107 | yes |
| QA-15 | MINOR | docs | Coverage table misses 9 of 26 test binaries; `TestPolygon2D` misnamed | TestingLocally.rst 165-250 | yes |
| QA-16 | MINOR | tests | UB `static_cast` downcast in a test; `looseMargin<float>` ≈ 0.012 absolute; `exactMargin()` defined three times | TestPolygon2D 38-52, TestFloatingPointUtils 40-45 | yes |
| QA-17 | MINOR | tests | Example checks: clock seeds, sum comparisons, assertion-only checks, MeshSDF sampling box, no checks in Shapes/OctreeBoundingVolume | Examples/*/main.cpp | yes |
| QA-18 | MINOR | hygiene | `CSGUnion` is dead `#if 0` code that CI builds and runs in five lanes | Examples/CSGUnion, CI.yml 132, 204 | yes |
| QA-19 | MINOR | design | CI example lists hand-maintained and drifted; no `-Werror`; example jobs build unused tests; examples force `-O3 -march=native` | CI.yml 132-204 | yes |
| QA-20 | MINOR | hygiene | Doxygen hook never triggers on `.hpp`; codespell names `Exec/`; Sphinx without `-W`; `CheckDocs.py` not in CI | .pre-commit-config.yaml 21, 37 | **R** |
| QA-21 | MINOR | design | Install/packaging advertised but absent; no `target_compile_features(cxx_std_17)` | CMakeLists 1-14, Building.rst 54-60 | yes |
| QA-22 | MINOR | design | `CI-passed` is skipped (not failed) when a dependency fails; CI never runs on push | CI.yml 3-7, 491-496 | partial |
| QA-23 | NIT | docs | Doc snippets use pointer-returning APIs; ConfigurationOptions steers users to `SignedDistanceFunction`; Quickstart runs an example that prints nothing | MemoryModel.rst 169-188, … | yes |
| QA-24 | NIT | hygiene | No include guard in EBGeometry.hpp; transitive includes in examples; GNUmakefile `?=` pitfalls; Doxygen shows `EBGEOMETRY_HOST_DEVICE` in return types; "130+ unit tests" (335) | various | yes |

---

## Appendix A — tape design inputs

Collected from the reviewers' design notes, for the tape design document.

**Primitive storage.** Shapes are already trivially copyable, so their `signedDistance()` can serve
as the single source of each formula. Store one `PODVector<S>` per primitive type and have a clause
hold (opcode, index), with no separate trait layer. Mesh primitives are layouts in the tape's pool
(§2.1). Loaders must never leave transient data in that pool (MESH-10).

**Two register kinds.** Point maps apply before their subtree is evaluated. Use Vec3 point
registers produced by point-map clauses, and scalar value registers (plus a `uint32` winner index)
produced by primitive and value-op clauses, rather than lowering to x/y/z scalar arithmetic. Fold
consecutive Translate/Rotate/Reflect/Scale into one affine clause (3×4 matrix + `|s|` value
multiplier) at compile time; add a general-axis rotation.

**Not purely linear.** BVH-union leaves (only unpruned children are evaluated), `Blur` (27 taps) and
`Mollify` (n³ taps) all need "evaluate sub-program k at point q". One CALL mechanism with a bounded
explicit stack covers all three; alternatively restrict Blur/Mollify to host-only tapes or unroll
up to a cap. Nested BVH traversal (a union of meshes) needs its own stack budget: bound nesting
depth, or use one framed stack per thread.

**Per-clause metadata computed by the compiler.**
- *Bounding volume.* Shapes: analytic, infinite for Plane/InfiniteCylinder/InfiniteCone/Perlin.
  Affine maps: transform the 8 corners. Elongate: grow by h. Offset: grow by max(r,0). Annular:
  grow by δ. Blur: grow by √3·d. Mollify: grow by √3·r. FiniteRepetition: the box swept over the
  tile range. Complement: infinite. Union: union of boxes (smooth union also grows by s/4
  polynomial, s·ln2 exponential). Intersection: intersection. Difference: A's box. Fallback: the
  octree-free bounding-volume approximation over the sub-program, on the host.
- *Distance quality*: exact / conservative (1-Lipschitz) / not a distance. BVH pruning is legal only
  above "not a distance".

**Operations.**

| Operation | Math | SDF? | Opcode |
|---|---|---|---|
| Complement | v' = −v | preserved | VNEG |
| Translate / Rotate / Reflect | affine point map | preserved | AFFINE |
| Scale | p' = p/s, v' = \|s\|·v (after CSG-1) | preserved, uniform only | AFFINE (+ value multiplier) |
| Offset | v' = v − r | exact for dilation, bound for erosion | VADDC |
| Annular | v' = \|v\| − δ | preserved | VABS + VADDC |
| Elongate | p' = p − clamp(p, −h, h) | exact outside only | PMAP_ELONGATE |
| FiniteRepetition | p'ᵢ = pᵢ − cᵢ·clamp(round(pᵢ/cᵢ), −loᵢ, hiᵢ) | only if the base fits its cell | PMAP_REPEAT |
| Blur | Σ w f(p + d·(i,j,k)) | 1-Lipschitz | CALL×27 + weighted accumulate |
| Mollify | Σ wₖ f(p − oₖ), non-negative kernel (after CSG-2) | 1-Lipschitz | CALL×n³ + accumulate |
| Union / Intersection | min / max (n-ary) | exact outside / inside | VMIN / VMAX |
| Difference | max(A, −min Bᵢ) | conservative | VNEG + VMIN + VMAX |
| Smooth union / intersection | blend of the two extreme values | 1-Lipschitz | SMIN2 / SMAX2 with blend kind as data |
| BVH (smooth) union | min / two-nearest blend over a `PackedBVH<T, uint32_t, K>` of sub-program ids | inherits | UNION_BVH / SUNION_BVH |
| Add / displace (missing today) | v' = v₁ + v₂ | not preserved | VADD (needed for noise) |

Note that n-ary smooth operations keep the "two extreme values" semantics, which differs from a fold
of binary smooth-min.

**Backend-neutral device code.** The interpreter, and everything it calls, stays within what SYCL
(and OpenMP offload) accept in device code, as the rest of the device code already does: no
recursion (explicit stacks, as BVH traversal and the octree walk use), no function pointers or
virtual calls, no exceptions, and only trivially copyable state passed to a kernel. Then adding a
backend after the tape (item 27) needs no change to the interpreter.

**Point clouds as a leaf.** A union of N equal spheres, or unsigned distance to a sample set, is
`sqrt(closestPoint(q).distanceSquared) − r`, much cheaper than a BVH union over sphere primitives.
If the tape should offer it, `PointCloudBVH` needs a uniform `signedDistance`-shaped entry and the
PC-4/PC-7 cleanup first.

**Gradients.** Decide whether the tape offers normals by forward-mode dual numbers over clauses
(one evaluation) or by finite differences (six).
