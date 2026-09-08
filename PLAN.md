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

## PR B — storage policies *(done)*

Reduce the mechanism to two POD policies. `SharedPtrStorage` is **purged**.

| Policy | Indirection | Dedup | Needs a canonical owner array | Crosses to device |
|---|---|---|---|---|
| `ValueStorage` (default) | none | no | — | yes |
| `IndexStorage` *(new)* | index | yes | yes | yes |

`IndexStorage<P>` is `StorageType = uint32_t`, resolved as `static_cast<const P*>(base)[idx]` through
`get(stored, base)`. It recovers what `SharedPtrStorage` was wanted for — one primitive set shared by
many BVHs, edited in one place — at 4 bytes instead of 16 plus a control block. What it does not
recover is automatic lifetime: the owning array must outlive every BVH indexing into it.

Purging `SharedPtrStorage` is what makes PR C possible: both remaining policies are trivially
copyable, so `m_primitives` can become a `PODVector` in *every* instantiation and `PackedBVH` needs
no second, permanently host-only backend.

### What the purge cost: BVH-accelerated CSG unions are compiled out

`BVHUnionIF`/`BVHSmoothUnionIF` keep their primitives alive through the primitive array of the
`PackedBVH` they own, storing them as `shared_ptr<const P>` where `P` is the **abstract** base
`ImplicitFunction<T>` in every real use (`Examples/CSGUnion`, `Examples/NestedBVH`,
`Tests/TestBVH.cpp`; concrete `Sphere<T>` only in `TestCSG`). Neither remaining policy can serve a
polymorphic primitive — `ValueStorage` would store an abstract type by value, and `IndexStorage`
indexes a flat array of one concrete type. There is no storage policy that fixes this; the fix is the
index-based redesign of the implicit-function/CSG layer (roadmap steps 4–5).

So the union classes and everything depending on them are **compiled out**, not deleted, behind a
single `EBGEOMETRY_ENABLE_BVH_CSG_UNION` guard in `EBGeometry_CSG.hpp`:

* `BVHUnionIF`, `BVHSmoothUnionIF`, `BVHUnion`, `BVHSmoothUnion`, and `CSGDetail::buildBVH`
  (`EBGeometry_CSG.hpp`, `EBGeometry_CSGImplem.hpp`).
* `Tests/TestCSG.cpp`'s whole BVH-union section; `Tests/TestBVH.cpp`'s nested-BVH case.
* `Examples/{CSGUnion,NestedBVH,PackedSpheres,RandomCity}` — the guard wraps each `main()` body, and
  the disabled branch prints why and exits 0, so the programs still build and `ctest` still runs them.
* `Integrations/{AMReX,Chombo}/{PackedSpheres,RandomCity}` — carry a header note only. They are
  illustrative, are not built or tested by CI, and will compile again unchanged.

Restoring is one line, plus a policy that can hold polymorphic primitives.

### Other changes in this PR

* `MeshSDF` → `ValueStorage<Face>`. The stale claim that it *could not* use `ValueStorage` (because
  `DCEL::FaceT`'s copy constructor was incomplete) was wrong in three places — `PackedBVH`'s
  copy-constructor comment, `ImplemBVH.rst`'s storage-policy section, and its mesh-SDF section — all
  corrected. `FaceT` became a plain trivially-copyable value in #137.
* `getClosestFaces` returns `std::vector<std::pair<uint32_t, T>>`. The index is into the **BVH's own
  primitive array**, which is what the traversal produces and the faithful translation of the old
  `shared_ptr` (which pointed at the BVH's copy, not the mesh's face). A mesh face index cannot be
  recovered under `ValueStorage` at all — packing reorders into leaf order and stores copies.
* The three direct constructors take `std::vector<std::pair<StorageType, BV>>` rather than
  `pair<P, BV>`. Identity for `ValueStorage`; it is what makes `IndexStorage` constructible, since
  those constructors partition on bounding volumes alone and never need the primitive.
* `pack()`/`packWith()` reject `IndexStorage` with a `static_assert`: a tree's leaves carry no index
  into any owning array.
* `refit()` gains a defaulted `const void* a_base` for `IndexStorage`.
* **New:** a mutable `getPrimitives()`. Under `ValueStorage` the packed BVH owns its primitives, so
  mutating the source no longer reaches it — this is the only way to move a packed geometry in place
  before `refit()`. The refit test caught this and now exercises the new path.

## PR C — pool-backed storage and the device view *(done)*

* All three arrays are `PODVector`s reserved from a caller-supplied `Pool`: `m_linearNodes`,
  `m_childAabbSoA`, `m_primitives`. Alignment needed no special handling (finding 5).
* Every construction entry point takes a `Pool&`: `pack`, `packWith`, the three direct constructors,
  the protected builder constructor, `PointCloudBVH`, and `TriMeshSDF`'s triangle-list constructor
  (which previously had no pool at all). `MeshSDF`/`TriMeshSDF`'s mesh constructors already had one.
* Construction still assembles into `std::vector` scratch and copies into the pool once, through a
  single `finalize()`. A `PODVector` never reallocates, and no build knows its node count up front;
  growth belongs in the host-only build step.
* `m_control` / `m_base` / `base()` / `rebasedView()` / `deepCopy()` / `isAttachedTo()` duplicate
  `DCEL::MeshT`'s pattern, as planned.
* `static_assert(std::is_trivially_copyable_v<PackedBVH<...>>)` at both precisions and under both
  storage policies.
* `pruneTraverse` is `EBGEOMETRY_HOST_DEVICE`, with the stack depth selected by compilation pass:
  256 on host, 64 on device.
* `getPrimitives()` returns a `PODSpan` (const and mutable), and `PackedLeafEvaluator` takes one.
* `TestBVH` is registered in `EBGEOMETRY_GPU_TESTS`, with a `[gpu]` case that launches a kernel over
  a rebased view, plus a host-to-host `rebasedView` case that runs everywhere.

### Two things worth knowing

**A copy is no longer a deep copy.** A pool-resident `PackedBVH` is descriptors plus two address
fields, so a copy aliases the original's pool memory. That shallowness is exactly what makes the type
trivially copyable and therefore mirrorable, but it inverts what the old copy constructor meant.
`deepCopy(Pool&)` is the replacement, and the copy test now asserts both halves.

**The `[gpu]` case has never been compiled.** This machine has an NVIDIA RTX A4000 but no CUDA
toolkit (`nvcc` is not installed), so neither the `cuda` nor the `hip` preset can be configured here.
The functors the kernel passes to `pruneTraverse` are hoisted out of the device-only guard and used
by the host rebase test, so the callable path is compiled and run on every build; the kernel launch
itself is the only unverified part. It needs a machine with a toolkit before this is trusted.

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
