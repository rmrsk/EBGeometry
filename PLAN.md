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

## PR B — storage policies *(done, then largely reversed — see PR D)*

Reduce the mechanism to two POD policies. `SharedPtrStorage` is **purged**.

> The `ValueStorage`/`IndexStorage` split described here shipped and was then removed again by PR D.
> What survives is the part that mattered: the `shared_ptr`-based primitive array is gone for good,
> and `PackedBVH` stores its primitives by value with no policy parameter. The rest of this section
> is kept as the record of how that was arrived at.

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

## PR D — the `IndexStorage` path: **delete the policy** *(done — this PR)*

`IndexStorage` shipped with PR B reachable from two of `PackedBVH`'s five constructors and used by
nothing. Investigating what it would take to finish it turned up, instead, that every justification
for it fails, and that two unrelated levers are worth far more. **So it was removed rather than
completed, along with the `StoragePolicy` parameter it was the only other argument for.** This
section is the evidence, kept so the decision does not have to be re-derived — and so a future
reader who wants the pattern back finds out first that `PackedBVH<T, uint32_t, K>` already is it.

Measurements below are a 135,200-face watertight torus, `T = double`, `K = 4`, `sizeof(FaceT)` = 88 B,
host, single-threaded.

### The crux: packing reorders, so the Pool cannot dedupe

Worth stating first because the `Pool` looks like it should already make this a non-issue. Packing
**reorders primitives into leaf order** — that is the property the packed representation exists to
create — so a packed BVH can never alias the array it was built from. `m_primitives` is a
`PODVector`, an offset rather than a pointer, so two BVHs *could* share one descriptor; but only if
they agree on leaf order, and two partitionings never do. Pool residency buys position independence
and lifetime, not a shared ordering. That leaves copy, index, or reorder-the-source, and no fourth
option — which is why a storage *policy* looked like the answer.

### Why each justification fails

**Sharing and instancing — already solved a layer up.** `Examples/NestedBVH` builds one `TriMeshSDF`
and instances it; its own comment says the inner BVH is "built and stored exactly once", shared by
`shared_ptr`. `TriMeshSDF`'s class documentation says instancing "is unaffected either way". And since
PR C a `PackedBVH` copy is shallow — measured on an 87,381-node BVH, a copy is **64 bytes with a pool
delta of zero**, aliasing the source; `deepCopy()` is opt-in and called nowhere in `Source/`,
`Examples/` or `Integrations/`. A BVH of 8 inner BVHs costs **1,856 bytes** in total, all 8 aliasing
the one original. The nesting case PR B named is the case the trivially-copyable descriptor already
solved: **a `PackedBVH` is itself an index** — three `PODVector` offsets plus a base.

**The CSG/tape layer — a `uint32_t` primitive is byte-identical.** Under the tape a union's
primitive is a clause id. `PackedBVH<T, uint32_t, K>` stores the same four bytes with no indirection,
and — `uint32_t` being an ordinary primitive type — it builds through all four construction paths:
SFC, partitioner/SAH, `ClusterSpec` and `TreeBVH::pack()`. `IndexStorage` would only move the indexing into the policy, for
nothing: `pruneTraverse` never calls `get()`. This confirms PR B's own wording, that the fix for the
compiled-out unions "is not another storage policy".

**`MeshSDF` deduplication — reproducible today, and beaten by a better fix.** `MeshSDF` retains
`m_mesh` *and* a `PackedBVH<Face>` of copies, so every face is stored twice. But the same saving is
available with no library change at all: `PackedBVH<T, uint32_t, K>` over mesh face indices, with a
leaf evaluator resolving `mesh.getFace(idx)`, was verified against `MeshSDF` over 81 queries at
**worst |delta| = 0** — exact agreement, default storage, no policy.

And it is beaten outright: **the BVH query path never reads the mesh's face array.**
`FaceT::signedDistance` resolves topology through `a_mesh.getEdge(...)` and `.getVertex(...)`; the only
query-shaped function touching `getFace` is `EdgeT::computeNormal`, called once at reconcile time
(`EdgeImplem.hpp:69`) and cached into `m_normal`. So after `pack()` the mesh's face array is dead
weight for a `MeshSDF`, and discarding it (build into pool A, compact into pool B, drop A) saves the
same ~11.9 MB **at zero query cost**, where `IndexStorage` makes every query slower.

### Why it is not a footprint win either

`IndexStorage` does not read less. The evaluator still needs the normal, centroid and projection axes,
so it touches all 88 bytes regardless:

| | bytes stored | bytes read per leaf of *n* faces |
|---|---|---|
| primitives stored by value | `n x 88` | `n x 88`, contiguous |
| `IndexStorage` | `n x 4` | `n x 4` contiguous **plus** `n x 88` scattered |

Its only effect is resident footprint, and even there it is third on the list. Ranked on the same
79 MB pool: zero-filled leaf SoA rows ~17.3 MB (22%, no downside); discarding the mesh's face array
~11.9 MB (15%, no query cost); `IndexStorage` 11.36 MB (14%, **every query slower**). Last, and the
only one that costs something.

Note also that `FaceT` is a materialised cache, not a fat primitive: 4 B of identity (`m_halfEdge`)
plus **64 B of derived geometry** (normal, centroid, area, two projection axes), 2 B metadata, a 4 B
policy enum and 14 B of padding. "88 versus 4" is the whole cache against the key to it.

### What was removed

Both policies went, not just `IndexStorage`: with one policy left, `StoragePolicy` would have been a
template parameter with exactly one legal argument, and every operation it abstracted over collapses
to something simpler when there is nothing to abstract.

* `BVH::IndexStorage` and `BVH::ValueStorage`, and with them `appendTreeLeaf`'s `static_assert` and
  the partitioner/leaf-predicate constructor's (`b042765`), which existed only to reject the former.
* The `StoragePolicy` template parameter on all five public surfaces: `PackedBVH`, `TreeBVH::pack`,
  `TreeBVH::packWith`, `TriMeshSDF`, and `Parser::readIntoTriangleBVH` (both overloads). The
  `StorageType` alias is gone too — `m_primitives` is a `PODVector<P>` and `getPrimitives()` returns
  a `PODSpan<P>`.
* `get()`, which was identity in the surviving policy. `appendTreeLeaf` and `appendAliased` became
  protected static helpers on `PackedBVH`, keeping `appendTreeLeaf`'s no-`reserve` comment — the one
  that prevents an O(N²) build, and has a regression test of its own.
* `refit`'s defaulted `const void* a_base`, which existed only to resolve `IndexStorage` indices.

The only source-level break is for a caller naming the removed template argument. In-tree that was
`TestBVH` alone; `MeshSDF`, `PointCloudBVH`, `TriMeshSDF` and both `Parser` entry points all used the
default and are unaffected.

`TestBVH`'s two agreement cases were kept and re-pointed: instead of `IndexStorage` versus
`ValueStorage`, they now check a `PackedBVH<T, uint32_t, K>` over indices against a
`PackedBVH<T, Pnt, K>` over the primitives themselves, still requiring bit-identical results, through
the SFC and `ClusterSpec` constructors. The SFC case additionally builds the same indices through
`TreeBVH::pack()` and requires that to agree too — the path `IndexStorage` could never take, since a
tree's leaves hold `shared_ptr<const P>` and carried no index for `appendTreeLeaf` to record. With
`P = uint32_t` there is nothing to record: the index *is* the primitive.

### What would bring it back

A consumer that needs genuine *sharing* — several BVHs with different partitionings over one primitive
set, editing a primitive once and having all of them see it — rather than deduplication. Nothing in
the tree wants that today, and the two candidates that looked like they might (nesting, CSG) turned
out not to. Note that even then the first thing to reach for is `PackedBVH<T, uint32_t, K>`, which
gives the sharing without any policy machinery; a policy would only be warranted if the index-resolve
step itself had to vary per instantiation.

## Bigger levers, unrelated to the storage policy

These came out of the same investigation, are independent of it, and are worth more. Same mesh and
build as above (the SFC constructor, so `targetLeafSize` is explicit).

| target leaf | nodes | pool | faces evaluated/query | coherent ns/query |
|---|---|---|---|---|
| **1** | 349,525 | 99.3 MB | 613 | 79,888 |
| 2 | 349,525 | 99.3 MB | 1,898 | 170,635 |
| **4** | 87,381 | 33.3 MB | 1,053 | **74,167** |
| 8 | 87,381 | 33.3 MB | 3,981 | 203,133 |
| **16** | 21,845 | 16.8 MB | 4,510 | 211,924 |
| 32 | 21,845 | 16.8 MB | 17,938 | 761,018 |
| **64** | 5,461 | 12.7 MB | 4,745 | 230,059 |

**1. A `targetLeafSize` that is not a power of K is a silent trap.** 2, 8 and 32 are each *strictly
dominated* by the power of K below them — identical node count and identical pool bytes, but 2 to 4
times the faces evaluated per query. The SFC build picks a level count, so the tree only changes at
powers of K; an in-between value keeps the same tree and packs more primitives into each leaf,
coarsening the pruning, with no warning. Rounding the argument to a power of K, or documenting it, is
small and self-contained.

**2. Leaf size moves total footprint further than any storage decision.** Going from leaf 1 to leaf 4
cut the pool **99.3 -> 33.3 MB (66%)** while getting slightly *faster* — 1.7x more faces evaluated,
won back on locality. That is roughly six times what `IndexStorage` could save, from a constructor
argument that already exists. For reference `MeshSDF`'s SAH build lands at ~119,925 nodes / 43.6 MB,
between leaf 1 and 4, because `DefaultLeafPredicate` stops at `< K` primitives.

**3. About 75% of the `ChildAABBSoA` cache is zeros.** `buildSoA` reserves one 192-byte row per node
and fills only interior ones, and `pruneTraverse` reads the SoA only in its non-leaf branch — so leaf
rows are allocated, zeroed and never read. In a K-ary tree where every interior node has exactly K
children, `N = 1 + K*I`, so at K = 4 leaves are 75% of nodes: **~17.3 MB of the 79 MB pool**, larger
than everything `IndexStorage` could save, and recoverable with no indirection or policy.

**Caveats.** One mesh, one `K`, one query pattern, host-only, single-threaded, and a torus is unusually
uniform so the SFC build flatters itself here. Finding 1 is structural and should generalise; the
"leaf = 4" figure is specific to this case and should be repeated on a real mesh under the SAH build
before becoming a default. Wall-clock on this machine was unreliable until cross-checked against the
deterministic faces-per-query count — treat the ns column as corroboration, not as a benchmark.

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

Primitives named by index needs no new BVH machinery: the union's BVH is a
`PackedBVH<T, uint32_t, K>`, four bytes per primitive with the index resolved in the leaf callback.
PR D above has the evidence, including why the `IndexStorage` policy that once wrapped this pattern
was removed rather than finished.

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
