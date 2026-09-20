# Plan: the BVH port, and what follows it

The pool-ergonomics work this document previously described as "PR 1" landed as
[#143](https://github.com/rmrsk/EBGeometry/pull/143); what survived of it is folded into
`PORTING.md`'s "current foundation" section, so it has been removed here.

The BVH port itself — PRs A, B and C below — is **done**, on branch `bvh_port_PR1`
([#145](https://github.com/rmrsk/EBGeometry/pull/145)), against `dev` at `e6914d4`. Those three
sections are kept as a record of what was decided and why, including several places where the plan
turned out to be wrong about the code. "What comes next" is the live part.

> **This is a working document with a finite life.** Once the mesh SDFs and the implicit-function
> layer have followed, fold whatever is still true into `PORTING.md` and delete this file — a plan
> that outlives its work becomes a second, stale source of truth.

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

## PR A — factor the traversal, give the scalar path a real implementation *(done)*

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

## PR D — the `IndexStorage` path *(planned; blocked on one measurement, and possibly on nothing else)*

`IndexStorage` shipped with PR B reachable from two of `PackedBVH`'s five constructors and used by
nothing. This section records what it is actually for — which is **not** what PR B's prose says — what
blocks it, and how much of it is worth building now.

### The crux is the reorder, not the Pool

Packing **reorders primitives into leaf order** so that a leaf scan walks contiguous memory. That is
the property the packed representation exists to create. The consequence is that a packed BVH can
never simply alias the array it was built from, because that array is in source order.

This is worth stating plainly because the `Pool` looks like it should already solve the problem and
does not. `m_primitives` is a `PODVector<StorageType>` — an offset, not a pointer — so two BVHs
*could* share one primitive array by holding the same descriptor, with no storage policy involved at
all. But only if they agree on leaf order, and two different partitionings never do. Pool residency
gives position independence and lifetime; it does not give a shared ordering.

So a packed BVH has exactly three options for its primitives, and there is no fourth:

1. **Copy them into leaf order** — `ValueStorage`. Contiguous leaf scans, `sizeof(P)` per primitive.
2. **Store indices in leaf order** — `IndexStorage`. 4 bytes per primitive, one scattered load per
   primitive evaluated.
3. **Reorder the source itself** to match leaf order. For a DCEL mesh that renumbers every face index
   the topology refers to. Not free, and not on any roadmap.

`IndexStorage` is therefore a **locality-versus-duplication trade**, exactly as
`EBGeometry_BVH.hpp`'s own policy comment says — not a sharing mechanism that the rest of the design
was missing.

### What PR B says it is for is wrong

PR B justifies the policy as "one primitive set shared by many BVHs — the `NestedBVH` case". Measured,
that case duplicates nothing and `IndexStorage` would save nothing:

* `Examples/NestedBVH` builds one `TriMeshSDF` and instances it; its own comment says the inner BVH is
  "built and stored exactly once", shared by `shared_ptr` at the SDF level.
* `TriMeshSDF`'s class documentation already says instancing "is unaffected either way".
* `PackedBVH`'s copy constructor has been shallow since PR C, and `deepCopy()` is called nowhere in
  `Source/`, `Examples/` or `Integrations/`. Copying a BVH duplicates nothing either.

The real shape is the mirror image: **one BVH over primitives that something else already owns.** The
duplication comes from packing by value while the owner keeps the source, not from copying or nesting.
Measured in-tree:

| | duplication today | `IndexStorage` helps? |
|---|---|---|
| **`MeshSDF`** | every face stored twice — it retains `m_mesh` *and* a `PackedBVH<Face>` of copies. `sizeof(FaceT<double,short>)` is 88 B against 4 B for an index | **yes — 22x on the BVH's half** |
| `PointCloudBVH` | ~2.6x (100k points: 3.20 MB retained + 5.07 MB packed groups = 82.7 B/point for 32 B of data) | no — its primitive is an AoSoA *group*, a SIMD reformatting of the points rather than a copy of one. See the pull-forward section below |
| `TriMeshSDF` | none | no — groups are built at pack time and the mesh is discarded |
| nesting / instancing | none | no — already shared a layer up |

`MeshSDF` cannot drop the mesh to avoid this: `FaceT` stores `m_halfEdge`, an index into the mesh's
edge array, so evaluating a face needs the mesh. Both really are required; it is the 88-byte face
*records* that are duplicated, not the edges and vertices.

**What those 88 bytes are matters for the trade below**, because a face is not an inherently fat
object — `FaceT` is a materialised cache:

| member | offset | size | |
|---|---|---|---|
| `m_halfEdge` | 0 | 4 | the face's only real identity — the index the mesh resolves |
| `m_normal` | 8 | 24 | cached geometry, derived from the mesh |
| `m_centroid` | 32 | 24 | cached geometry, derived |
| `m_metaData` | 56 | 2 | user payload |
| `m_area` | 64 | 8 | cached geometry, derived |
| `m_xDir` / `m_yDir` | 72 / 76 | 4 + 4 | cached 2D-projection axes, derived |
| `m_insideOutsideAlgorithm` | 80 | 4 | per-face policy enum |

74 B of members plus 14 B of padding. **64 of the 88 are precomputed geometry**, all recoverable from
`m_halfEdge` and the mesh. So "88 versus 4" is the whole cache against the key to it — not a fat
primitive against a thin one. The cache itself is not waste; it is what keeps `signedDistance` from
recomputing a normal per query. The defect is only that `MeshSDF` holds two copies of it.

Two unrelated savings are visible in that layout and are worth taking regardless of what happens to
`IndexStorage`, because both shrink the number that would make it attractive: the 14 B of padding is
16% of the struct in *both* copies and member reordering recovers about 8 B of it, and
`m_insideOutsideAlgorithm` is a per-face 4-byte enum that is almost certainly uniform across a mesh
and belongs on the mesh.

There is also a non-memory payoff for `MeshSDF`: under `IndexStorage` the indices would point into the
mesh's own face array, so `getClosestFaces` could return a **mesh** face index instead of the BVH-local
one PR B had to document as unrecoverable — the wart that forced `Integrations/AMReX/PaintEB` to be
rewritten.

### The CSG layer does not need it: `ValueStorage<uint32_t>` already does that job

This was the second justification for the policy, and it does not survive either.

Under the tape a BVH union's primitive is a **clause id**. The natural reading is that a `uint32_t`
primitive means `IndexStorage` — but `ValueStorage<uint32_t>` has `StorageType == P == uint32_t` and
stores exactly the same four bytes, with no indirection. Verified against the tree: a
`PackedBVH<T, uint32_t, K>` with the **default** policy builds through all four construction paths —
the SFC, partitioner/SAH and `ClusterSpec` constructors *and* `TreeBVH::pack()` — and is trivially
copyable. None of PR D's work is needed for any of it.

The distinction between the two is semantic, not physical:

* `ValueStorage<uint32_t>` — *the primitive is a clause id.*
* `IndexStorage<P>` — *the uint32 is an index into an array of `P`*, and `get(id, base)` returns a
  `P&`.

For the tape the clause id genuinely **is** the primitive: there is no array of `ImplicitFunction<T>`
to index into, because a tape is a clause list an interpreter walks, not an array of objects. So
`ValueStorage<uint32_t>` is the more honest model, and `IndexStorage` would only relocate the
indexing into the policy — for nothing, since `pruneTraverse` never calls `get()`.

**Consequence: re-enabling `EBGEOMETRY_ENABLE_BVH_CSG_UNION` is not blocked on this section at all.**
It is blocked on step 1's de-virtualisation and step 3's tape, which is what PR B's own prose already
said ("the fix is not another storage policy").

### Is it worth building? One measurement decides

After both justifications fall away, `IndexStorage` has exactly **one** candidate consumer left:
`MeshSDF`, the measured 88-bytes-twice case above. Everything therefore hangs on a question nobody has
answered:

> **Does halving the resident footprint pay for scattering every leaf read?**

Note the shape of that question, because the obvious phrasing — "4 bytes scattered versus 88 bytes
contiguous" — is wrong, and the layout above shows why. The leaf evaluator still needs the normal,
centroid and projection axes, so it still touches all 88 bytes either way:

| | bytes stored | bytes read per leaf of *n* faces |
|---|---|---|
| `ValueStorage` | `n x 88` | `n x 88`, contiguous |
| `IndexStorage` | `n x 4` | `n x 4` contiguous **plus** `n x 88` scattered |

`IndexStorage` does not read less. It reads **the same data, less contiguously, with an index array on
top** — so per query it is strictly worse. Its only win is footprint: half the resident bytes for the
whole mesh, which pays off only when the working set is the binding constraint and the smaller
footprint keeps more of the *mesh* in cache. That is a second-order effect, and it sets a high bar for
the measurement:

* the mesh must be large enough that `88 B x faces` genuinely does not fit — a benchmark where both
  representations fit comfortably measures nothing;
* the query pattern must be the coherent cell-by-cell sweep the real consumers (AMReX/Chombo EB
  generation) perform, not random points, which would overstate the scatter cost;
* and it should be run after the padding and per-face-enum savings above, since those move the
  footprint ratio the whole argument rests on.

On the evidence so far the expectation should be that `MeshSDF` stays on `ValueStorage`. Measure
before writing any of the code below.

**If the measurement favours `IndexStorage` for `MeshSDF`:**

*Phase 1 — retype the construction machinery on `StorageType`.* The partitioner/leaf-predicate
constructor wraps every input in `shared_ptr<P>` and builds a stack-local `TreeBVH<T, P, BV, K>` probe
per split, to reuse the `Partitioner` and `LeafPredicate` contracts unchanged — both typed on `P`.
Retyping that machinery (`PrimAndBVList<StorageType, BV>`, `Partitioner<StorageType, BV, K>`,
`LeafPredicate<T, StorageType, BV, K>`, a `TreeBVH<T, StorageType, BV, K>` probe) is
**source-compatible for every existing caller**, since `StorageType == P` under `ValueStorage`. What
it unlocks that matters is **`pack()`/`packWith()`** — a tree built over `StorageType` *does* carry
indices in its leaves, so `IndexStorage::appendTreeLeaf` becomes implementable rather than a
`static_assert` — and `MeshSDF` builds through `pack()`. (Of the three shipped partitioners only
`PrimitiveCentroidPartitioner` reads the primitive; `BVCentroidPartitioner` and
`BinnedSAHPartitioner` read bounding volumes only and work over indices verbatim.)

*Phase 2 — the pool-resident owner array.* The caller reserves a `PODVector<P>` from the **same
`Pool`** and `get()` resolves through the BVH's own `base()`. That is the only option that works on
device without hand-patched pointers — the descriptor is an offset, so `Pool::mirror` relocates it and
`rebasedView()` rebases it with everything else — and it ties the owner array's lifetime to the Pool,
which an index otherwise cannot do. An external caller-owned array (what PR B shipped) leaves both
problems standing; a BVH-owned array defeats the sharing entirely. Note the cost is borne by
everybody: `PackedBVH` must keep one layout across both policies and stay trivially copyable, so the
owner descriptor is carried unconditionally — 16 bytes in every BVH including every `ValueStorage`
one, mirrored to device on every view.

**If the measurement does not favour it**, `IndexStorage` has no consumer, and the right move is to
delete it rather than carry a policy, two `static_assert`s and a documented asymmetry for nobody. What
would be lost is a type-level way to say "this uint32 indexes an array of `P`" — documentation, not
capability.

### Still open

* **What a partitioner may inspect under `IndexStorage`.** Either document that partitioners must be
  geometry-agnostic and `static_assert` `PrimitiveCentroidPartitioner` out of the combination, or widen
  the contract to carry a base so it can resolve. The first is cheap and honest; the second is uniform
  at the price of a public callback signature.
* **Whether `IndexStorage` earns its place at all.** This is now the section's real question, and the
  `MeshSDF` measurement answers it: that is the only candidate consumer left. Keeping a policy, two
  `static_assert`s and a documented constructor asymmetry for a feature nothing uses is a cost with no
  payer.
* **Whether the CSG layer should say `ValueStorage<uint32_t>` explicitly** when it lands, rather than
  leaving a bare `PackedBVH<T, uint32_t, K>` for the next reader to mistake for an oversight. The
  default is already correct; the intent is what is missing.

## What comes next

PRs A–C are done and are what this branch contains. The rest of the port proceeds in the order
below. The shape is: make each layer a concrete, self-contained, trivially-copyable type that a tag
can name, layer by layer, and only then build the tape that dispatches over those tags.

### 0. Prerequisite: close the verification gap

**There is no CUDA toolkit on the development machine** (an RTX A4000 is present; `nvcc` is not
installed), so neither the `cuda` nor the `hip` preset can be configured, and every `EBGEOMETRY_HOST_DEVICE`
annotation added from here lands unverified. This is not hypothetical: `PackedBVH::getBoundingVolume`
and `computeBoundingVolume` were missing their annotations and the `[gpu]` test's own kernel calls
one of them, so that case could never have compiled. It was caught by reading the code, not by
building it.

Everything below adds several times more device code than PRs A–C did. Install a toolkit before
starting, or the port accumulates device code that nobody has compiled.

### 1. The mesh distance functions, as standalone types

**This is two changes, not one, and the second is the one that gets forgotten.**

* **De-virtualise.** Give each class a concrete, non-virtual `EBGEOMETRY_HOST_DEVICE signedDistance()`,
  and let the existing virtual override become a one-line delegate to it. No API break at all. This
  is exactly the pattern `PORTING.md`'s roadmap step 4 prescribes for the analytic SDFs.
* **Drop the `shared_ptr` members.** `FlatMeshSDF` holds `shared_ptr<Mesh> m_mesh`; `TriMeshSDF` holds
  `shared_ptr<Root> m_bvh`; `MeshSDF` holds both. Annotating every method changes nothing while those
  members remain: the class is not trivially copyable and cannot be mirrored. They must become
  by-value members.

The second half is **newly possible because of this branch**: `PackedBVH` is now `static_assert`-ed
trivially copyable, and `DCEL::MeshT` has been verified to be so as well. Before PR C, holding either
by value was not an option.

Order, ascending by number of moving parts:

1. **`FlatMeshSDF`** — one member, no BVH. The smallest complete instance of the pattern, and porting
   it yields a *device-side brute-force oracle* to validate the other two against. Best value first.
2. **`TriMeshSDF`** — one member; its `TriangleAoSoA` primitives are self-contained.
3. **`MeshSDF`** — two members which must be rebased onto the *same* mirrored pool consistently. The
   only genuinely new problem in the group, so do it last.

`getClosestFaces` stays host-only: it runs on `traverse()`, which keeps its `std::function` interface.

**Define the tag/opcode registry once, here**, even though only three types populate it at first.
`PORTING.md` already calls for a generated opcode registry as the single source of truth; if the mesh
SDFs get an ad-hoc scheme now and the implicit functions get the real one in step 2, the two have to
be merged later.

Keep `ImplicitFunction<T>` and `SignedDistanceFunction<T>` alive throughout as the compatibility
surface. The first attempt deleted `SignedDistanceFunction<T>` outright, and that was a user-visible
break with no GPU motivation of its own.

### 2. The implicit-function and CSG layer

The same treatment, applied to the analytic SDFs, transforms and combinators: formula into a trait as
a `static EBGEOMETRY_HOST_DEVICE eval()`, virtual `value()` as a thin delegate, primitives named by
index rather than held by `shared_ptr`.

Primitives named by index means `IndexStorage`, whose remaining work — and the two decisions that
constrain how this step names its primitives — is set out in PR D above. Settle those before starting
here.

**Acceptance test:** re-enable `EBGEOMETRY_ENABLE_BVH_CSG_UNION` and get `TestCSG`'s union section and
the four disabled examples back to green. That turns "the CSG layer is index-based now" into a
pass/fail rather than a judgement call, and it repays the debt PR B took on.

### 3. The tape

Last, as planned: the linear-SSA clause list and interpreter, built on `Pool`/`PODVector` from the
start. It depends on step 2 for its opcodes and on PR C for the BVH-union opcodes, which reference
the packed BVH arrays directly.

### 4. Loose ends

`Triangle<T, Meta>` (AoS), `Octree`, `PointCloudHashGrid`, `SFC`, and the device-side point-cloud
*build* (Morton codes + radix sort). None of these blocks anything, so they genuinely can wait.

## Pull forward: `PointCloudBVH` should consume a PackedBVH, not derive from one

Not urgent in the sense that nothing blocks on it, but it should be done **before** step 1 rather
than left with the loose ends, for two reasons: PR C introduced a live bug there, and fixing it
establishes the exact pattern step 1's hardest case needs.

**The bug.** `PackedBVH`'s destructor documentation says it "is not intended to be subclassed or used
polymorphically", and `PointCloudBVH` publicly subclasses it anyway. That was a documentation
inconsistency until PR C added `rebasedView()` and `deepCopy()`, both of which return `PackedBVH`
**by value**. On the derived type they are therefore silently sliced. Verified by compiling a probe:

```cpp
auto rebased = pointCloud.rebasedView(pool);  // compiles; type is PackedBVH, not PointCloudBVH
auto deep    = pointCloud.deepCopy(pool);     // same
static_assert(!std::is_trivially_copyable_v<PointCloudBVH<double>>);        // holds
static_assert( std::is_trivially_copyable_v<PointCloudBVH<double>::Base>);  // holds
```

Mirroring a point cloud to a device therefore compiles cleanly and produces a BVH whose leaves refer
to a cloud that was never copied. Public inheritance additionally exposes the whole `PackedBVH`
surface — `refit()`, `traverse()`, and the mutable `getPrimitives()` that lets a caller reorder the
SoA groups out from under `m_order`.

**It is not an is-a relationship** in the first place. A `PointCloudBVH` owns a cloud
(`m_positions`, `m_metadata`) and a seeding table (`m_order`, `m_leafOff`, `m_leafCnt`), and *uses* a
BVH to index it. Its public API — `closestPoint`, `nearestNeighbor`, the brute-force references — has
no overlap with the BVH's.

The work, one pass over one file:

1. Hold `PackedBVH m_bvh` by value instead of deriving (it is trivially copyable now).
2. Cloud arrays (`m_positions`, `m_metadata`, `m_order`, `m_leafOff`, `m_leafCnt`) become `PODVector`s
   via the same `finalize()` pattern PR C established.
3. Its own `base()` / `rebasedView()`, rebasing the BVH member and the cloud arrays together;
   `static_assert` the whole class trivially copyable.
4. Delete the protected `PackedBVH(Pool&, nodes, prims)` constructor, which exists only to serve this
   derivation, and mark `PackedBVH` `final` — turning "not intended to be subclassed" from a comment
   into something the compiler enforces. **Check first** that no other subclass exists, including in
   `Integrations/` and downstream users.

Step 3 there is the same BVH-plus-payload rebase that `MeshSDF` will need in step 1, which is the
other reason to do it first.

## Documentation (done with PRs A–C)

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

Every PR: the full debug suite plus `debug-san`, `release-test`, `examples`, and the SIMD matrix
(`EBGEOMETRY_SIMD=none|sse41|avx|avx512`) for anything touching traversal.

Device coverage is currently **not** obtainable on this machine: there is no CUDA toolkit installed,
so neither `cmake --preset cuda` nor `--preset hip` configures, and the `[gpu]` cases cannot be
compiled, let alone run. This is the prerequisite recorded at the top of "What comes next". CI
compiles both backends but has no GPU, so even once a toolkit is available a green CI lane means "it
compiles" and nothing more — running the device tests locally before pushing is not optional.

## Sequencing note

PRs A–C landed together on `bvh_port_PR1` rather than as three PRs, at the maintainer's request. They
are separate commits and are reviewable in order; PR A in particular stands entirely on its own (host
only, no API change, covered by the existing `TestBVH`, and a CPU speedup in its own right).
