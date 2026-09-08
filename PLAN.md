# Plan: the BVH port

The pool-ergonomics work this document previously described as "PR 1" landed as
[#143](https://github.com/rmrsk/EBGeometry/pull/143); what survived of it is folded into
`PORTING.md`'s "current foundation" section, so it has been removed here. What remains is the BVH
port, expanded from the sketch that accompanied it and revised against the code as it stands at
`e6914d4`.

> **This is a working document with a finite life.** When the PRs below have landed, fold whatever
> is still true into `PORTING.md` and delete this file — a plan that outlives its work becomes a
> second, stale source of truth.

## Findings that revise the earlier sketch

Four things turned up on re-reading the code that the earlier PR-2 sketch either got wrong or did
not know.

### 1. The stated blocker on `ValueStorage` for `MeshSDF` is stale

`EBGeometry_BVH.hpp`'s `PackedBVH` copy-constructor comment says `ValueStorage` is "safe as long as
the primitive type's own copy constructor is complete; see `DCEL::FaceT`'s copy-constructor
documentation for a case where it deliberately is not, which is why `MeshSDF` never uses
`BVH::ValueStorage`". That is no longer true, and two other places in the tree already say so:

* `EBGeometry_DCEL_Face.hpp:118-125` now documents the copy constructor as "Defaulted memberwise
  copy: every member is a plain value ... immediately usable for a point-in-face test against the
  same mesh the source face belonged to."
* `EBGeometry_MeshDistanceFunctions.hpp:145` describes each packed face as "a fresh copy of the
  corresponding DCEL mesh face (`DCEL::FaceT` is a plain, trivially-copyable value, so copying it is
  cheap and free of aliasing)".

The incompleteness was `Polygon2D`, removed in #137. So `MeshSDF`'s primitives are *already* copies;
`SharedPtrStorage` currently buys a heap allocation and a pointer chase per face and nothing else.
This strengthens the case for `ValueStorage` as `MeshSDF`'s policy from "matches today's semantics"
to "strictly better, and semantically identical". **The stale comment must be corrected in the same
PR that relies on it being false.**

### 2. `Node` and `ChildAABBSoA` carry no host/device annotations at all

Every `Node` member function (`setBoundingVolume`, `getChildOffsets`, `isLeaf`,
`getDistanceToBoundingVolume2`, …) is a plain `inline`. Annotating them is an unlisted work item,
small but a prerequisite for any device traversal.

### 3. The seven traversal copies differ in exactly one expression

`pruneTraverse` is six `if constexpr` SIMD blocks plus a scalar fallback. Comparing them line by
line, the *only* difference between the six is how `dist2[K]` is computed from `(m_childAabbSoA
[idx], a_point)`. Stack handling, the pop, the `entry.dist2 > curBest2` prune, the leaf call, the
sort by descending distance, the re-read of `a_pruneDist2` and the push loop are identical to the
character. Factoring on a child-distance evaluator is therefore mechanical, not a redesign.

### 4. The scalar fallback is a different algorithm, not just a slower one

It does not share the loop at all: it builds four `std::function`s and delegates to `traverse()`,
which runs on a heap `std::vector` stack. Giving it the shared loop is a **behaviour change on the
scalar path** — the answer should be identical, but leaf-visit order and pruning timing need not be.
This has to be verified explicitly rather than assumed, and it is the main risk in PR A.

### 5. SoA alignment survives the pool migration for free

`ChildAABBSoA` is `alignas(sizeof(T)*K)` — 64 B for the two AVX-512 configurations. `Pool::reserve`
takes an alignment and aligns the returned offset (`PoolImplem.hpp:117`), `PODVector::reserveFrom`
passes `alignof(T)` through (`PODVector.hpp:139`), and the block base is `PoolBaseAlign` = 256 B. So
a pool-resident SoA array is still correctly aligned for `_mm512_load_pd`, with no special handling.
The only limit is `alignof(ChildAABBSoA) <= 256`, i.e. `sizeof(T)*K <= 256`, which an
`EBGEOMETRY_EXPECT` in `Pool::reserve` already enforces — unreachable for any realistic K.

## PR A — factor the traversal, give the scalar path a real implementation

Host-only. No API change, no `Pool`, no storage change. Independently useful as a CPU speedup for
any `(T, K)` with no compiled ISA path.

* Factor the branch-and-bound loop **once**, parameterised on a child-distance evaluator that fills
  `T dist2[K]` from `(const ChildAABBSoA&, const Vec3T<T>&)`. Each ISA block shrinks to the handful
  of intrinsics it actually contributes; the device path later becomes an eighth evaluator rather
  than an eighth transcription.
* Give the scalar path a real implementation: fixed stack, hand-rolled insertion sort over the ≤K
  children. No `std::function`, no heap, no `std::sort`/`std::clamp` (both drag in host-only
  libstdc++ helpers under hardened mode). Insertion sort is also simply faster at K ≤ 16.
* Annotate `Node` and `ChildAABBSoA` `EBGEOMETRY_HOST_DEVICE` (finding 2).
* Make stack depth a parameter, defaulting to 256 (today's value) on the host. The device value (64)
  arrives with PR C; `StackEntry` is 16 B at `double`, so `[256]` would be 4 KB/thread, and
  `log_K(N)·K` says 64 covers a million primitives at K=4.

`traverse()` and `refit()` stay exactly as they are — host-only, `std::function`-based, and still
used by `MeshSDF::getClosestFaces`.

**Verification.** `TestBVH` must pass unchanged under all four presets. Because of finding 4, add a
case that runs the same queries through the scalar path and a SIMD path and requires bit-identical
results — currently nothing pins the two together, and this PR is precisely where they could
silently diverge. Build the SIMD matrix explicitly (`EBGEOMETRY_SIMD=none|sse41|avx|avx512`) rather
than trusting the default preset.

## PR B — storage policies

Host-only. Reduce the mechanism to two POD policies; `SharedPtrStorage` is purged.

| Policy | Indirection | Dedup | Needs a canonical owner array | Crosses to device |
|---|---|---|---|---|
| `ValueStorage` | none | no | — | yes |
| `IndexStorage` *(new)* | index | yes | yes | yes |

`IndexStorage<P>` is `StorageType = uint32_t`, resolved as `static_cast<const P*>(base)[idx]`. It
recovers what `SharedPtrStorage` was actually wanted for — one primitive set shared by many
translated copies, edited in one place — at 4 bytes instead of 16 plus a control block. What it
cannot recover is automatic lifetime: the owning pool must outlive every copy, by convention rather
than refcount.

Purging `SharedPtrStorage` is what makes PR C possible at all: both remaining policies are trivially
copyable, so `m_primitives` is a `PODVector` in every instantiation and `PackedBVH` needs no
`std::vector` fallback backend.

Work items:

* `get()` gains a base parameter: `get(const StorageType&, const void* a_base)`. `Value` ignores it.
  Three call sites (`MeshDistanceFunctionsImplem.hpp:522,559`, `BVHImplem.hpp:1635`).
* Move the four default-policy sites from `SharedPtrStorage` to `ValueStorage`: `BVH.hpp:278`
  (`PackedBVH`), `:347` (`PackedLeafEvaluator`), `:1160` (`TreeBVH::pack`), `:1178` (`packWith`).
* `MeshSDF` → `ValueStorage`; `TriMeshSDF` already is. Correct the stale copy-constructor comment
  (finding 1) and the policy prose at `MeshDistanceFunctions.hpp:145` and `Parser.hpp:260`.
* `SharedPtrStorage::appendAliased`'s aliasing-constructor trick and the comment at
  `BVHImplem.hpp:506` go with it.
* `TreeBVH` keeps its own `shared_ptr` storage and `PrimitiveList<P>` — it is the host-only builder
  and is not policy-parameterised. Packing dereferences into values, which
  `ValueStorage::appendTreeLeaf` already does.

### `getClosestFaces`, and which index space it returns

`MeshSDF::getClosestFaces` returns `std::vector<std::pair<std::shared_ptr<const FaceT>, T>>`
(`MeshDistanceFunctionsImplem.hpp:316`) and must change. The earlier sketch left the index space
open; it resolves as follows.

The returned `shared_ptr` today points at *the BVH's own copy* of the face, not at the mesh's face.
The faithful translation is therefore an index into the BVH's primitive array, and that is also what
the traversal naturally produces. A *mesh* face index cannot be recovered under `ValueStorage` at
all — packing reorders primitives into leaf order and stores copies, so nothing remembers where a
face came from.

So: **return `std::vector<std::pair<uint32_t, T>>` indexing the BVH primitive array**, with the
accessor to resolve it. The more useful mesh-face-index version becomes natural once `IndexStorage`
lands, because there the stored `uint32_t` *is* the owner-array index — and that is a good reason to
land `IndexStorage` in this PR even though `MeshSDF` does not default to it.

### Open wrinkle: filling `IndexStorage` from a `TreeBVH`

A tree holds `shared_ptr<const P>` with no notion of the owner's array index, so `appendTreeLeaf`
cannot produce indices from it. Either have `MeshSDF` build the tree over face indices directly, or
give the packing path an explicit primitive→index mapping. This gates only `IndexStorage`'s build
path, not `ValueStorage`'s, so it can be settled inside this PR without blocking anything else.

## PR C — pool-backed storage and the device view

* All three arrays become `PODVector`: `m_linearNodes`, `m_childAabbSoA`, `m_primitives`. `Node` and
  `ChildAABBSoA` are already trivially copyable, so only the containers change. Alignment needs no
  special handling (finding 5).
* Every construction entry point gains a `Pool&`: `pack(Pool&)`, `packWith(Pool&, converter)`, the
  three direct `primsAndBVs` constructors, and the protected constructor `PointCloudBVH` uses.
  `MeshSDF` and `TriMeshSDF` already take a `Pool&`, so their signatures are unchanged and the mesh
  and its BVH land in one pool — one `mirror()`, one base, one `rebasedView`.
* Add `m_control` / `m_base` / `base()` / `rebasedView(const Pool&)`, duplicating `DCEL::MeshT`'s
  pattern rather than introducing a CRTP mixin (~15 lines; a base class complicates the
  trivial-copyability `static_assert`s for no real saving).
* `static_assert(std::is_trivially_copyable_v<PackedBVH<…>>)`, and on `StorageType`.
* Device stack depth 64; add the device child-distance evaluator as the eighth evaluator.
* Device callables: we compile with `--expt-relaxed-constexpr` but not `--extended-lambda`, so
  device callers must pass functors, not lambdas. Keep the templated API for that, and additionally
  ship the two common queries (signed distance, closest point) as ready-made device entry points so
  ordinary users never write a callback.
* `TreeBVH` stays host-only and unchanged — it is the builder, and static geometry builds on the
  host. `refit()` likewise stays host-only; nothing yet needs moving geometry resident on device.
* Add `TestBVH` to `EBGEOMETRY_GPU_TESTS` (`Tests/CMakeLists.txt:101`) and add a `[gpu]`-tagged case
  that launches a kernel and compares against the host, following `TestDCEL.cpp:1491`.
* Add a **host-to-host** `rebasedView` case: build a BVH, `mirror()` its pool into a second host
  pool, rebase, require identical query results. This is the only way the rebase invariant executes
  in CI, which has no GPU.

## PR D — the SDF wrappers on device

`TriMeshSDF` first (its `TriangleAoSoA` primitives are self-contained), then `MeshSDF` (whose faces
resolve topology against the retained `DCEL::MeshT`, so the mesh and the BVH must be rebased onto
the same mirrored pool). `getClosestFaces` stays host-only — it runs on `traverse()`, which keeps its
`std::function` interface.

This closes five rows of `PORTING.md`'s "What is not" table and unblocks the `BVHUnionIF` /
`BVHSmoothUnionIF` exception in roadmap step 4.

## Documentation

Per `CLAUDE.md`, in the same PRs, not after:

* `ImplemBVH.rst` (and any Concepts-section counterpart) for the storage-policy change and the
  pool-backed construction signatures.
* `MemoryModel.rst` gains `PackedBVH` alongside `DCEL::MeshT` as a pool-resident type.
* `PORTING.md`: move `TreeBVH`/`PackedBVH` and the mesh SDFs from "What is not" to "What is
  ported", and record the `m_control`/`m_base`/`rebasedView` duplication as the sanctioned
  convention for a second pool-resident class.
* `PORTING.md`'s roadmap ordering: the DCEL reconcile chain is listed as step 1 ahead of the BVH.
  It is independent, has no device caller today (`MeshT::reconcile`'s only production call site is
  `SoupImplem.hpp:226`, and `Soup` is host-only by design), and is being deferred deliberately. Say
  so there rather than leaving the document asserting a precedence we have decided against.
* Doxygen `@param`/`@return` on every signature that gains a `Pool&` or changes its return type.

## Verification

Every PR: the full debug suite plus `debug-san`, and the SIMD matrix for PR A. PRs C and D
additionally need `cmake --preset cuda -DCMAKE_CUDA_ARCHITECTURES=86` and `ctest -L gpu-device` on
the local RTX 3080 Ti. CI compiles both backends but has no GPU, so a green GPU lane means "it
compiles" and nothing more — running the device tests locally before pushing is not optional.

## Sequencing note

PR A is worth landing on its own even if the rest slips: it is host-only, has no API change, is
fully covered by the existing `TestBVH`, removes the `std::function`/heap-stack fallback that is the
single largest blocker in `PORTING.md`'s table, and is a CPU speedup in its own right.
