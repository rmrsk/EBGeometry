# Plan: pool ergonomics, then the BVH port

Two PRs. PR 1 removes the freeze/bind burden from users and is a prerequisite for PR 2, which
ports the BVH. The BVH design is settled below so PR 1 does not paint us into a corner.

> **This is a working document with a finite life.** It exists so the design decisions behind these
> two PRs are reviewable before any code is written. When both have landed, fold whatever is still
> true into `PORTING.md` and delete this file — a plan that outlives its work becomes a second,
> stale source of truth.

## Background: the problem PR 1 solves

`Pool::reserve` can grow, which moves the base, which invalidates any cached base. Today that is
handled by `freeze()`: seal the pool, then `bind()` each object. The cost is that **an object
cannot be queried until the whole pool is sealed**, so any workflow that keeps building — read ten
STLs, deep-copy one, translate a few, read another — collides with a lifecycle decision that was
only ever about pointer hygiene. The workarounds are already visible in the tree:
`Examples/MeshSDF` uses three pools because `FlatMeshSDF`/`MeshSDF` constructors freeze, and
`Parser::readIntoMesh(files, pool)` is "deliberately not a loop over the single-file overload" for
the same reason.

Rejected alternatives, and why:

- **Handle/descriptor split** (host handle owns `Pool&`, POD descriptor crosses): works, but two
  types and a visible API break.
- **Reserve address space up front so the base never moves**: users legitimately want several
  pools (a work pool and a final pool), and pools get created ad hoc deep inside third-party code.
  A guarantee that holds only when someone sized the pool correctly is not a guarantee.
- **Leave it, document a user-side `EBGEOMETRY_EXPECT(pool.base() == sdf.base())`**: only fires
  where someone remembered to write it. The library can do this check itself.

## PR 1 — pool-resident objects resolve through their pool

### The rule

A pool-resident type carries **both** pointers:

```cpp
const PoolControl* m_control = nullptr;   // host-only. null in, and only in, a device view.
void*              m_base    = nullptr;   // set on the host by rebasedView; read only on device.

EBGEOMETRY_HOST_DEVICE const void* base() const noexcept
{
#if defined(EBGEOMETRY_DEVICE_COMPILE)
  EBGEOMETRY_EXPECT(m_control == nullptr);   // a host descriptor reached a kernel without rebasedView
  return m_base;
#else
  EBGEOMETRY_EXPECT(m_control != nullptr);   // a device view is being dereferenced on the host
  return m_control->m_base;
#endif
}
```

The two are not redundant: `m_base` points *into the data block*, which `mirror()` copies, so it
has a device counterpart. `m_control` points at *host bookkeeping* that is never mirrored and has no
device counterpart. Host resolution always goes through the control block and is therefore immune to
growth; device resolution uses the snapshot base, because a kernel cannot follow a control block.

Both assertions are exact, not best-effort, because `rebasedView` (below) is the only producer of a
null `m_control` and it produces one *only* for a device-accessible target. So `m_control == nullptr`
means "device view" and nothing else, in both directions:

- On device, a non-null `m_control` means a host descriptor was copied into a kernel directly.
- On host, a null `m_control` means either a device view being dereferenced on the host, or a
  default-constructed mesh nobody ever reserved into. Both are errors, caught at the point of use.

Note the consequence for `m_base`: on the host it is **write-only**. Nothing reads it until the
descriptor has been byte-copied into a device address space, which makes it look dead to a host-only
reader and needs saying on the field itself.

The type stays trivially copyable (two raw pointers plus `PODVector`s), so every existing
`static_assert` and `Meta` requirement holds unchanged.

### Why a control block rather than a `Pool*`

The obvious form of the above stores `Pool* m_pool` and resolves through `m_pool->base()`. It works,
but it regresses a property the code has today. `MeshT::m_base` currently holds the address of the
*block*, and `Pool`'s move constructor steals that pointer verbatim (`PoolImplem.hpp:58-68`) — the
block does not move, only its owner does — so a bound mesh survives its pool being moved. Storing a
`Pool*` points the mesh at precisely the thing a move relocates:

```cpp
std::vector<Pool> pools;                  // one pool per geometry
pools.emplace_back(hostMemoryResource());
auto mesh = Parser::readIntoMesh<T, Meta>(file, pools[0]);
pools.emplace_back(hostMemoryResource()); // reallocates: pools[0] is moved, then destroyed
mesh->signedDistance(p);                  // reads freed vector storage
```

That snippet is correct against today's code and would silently break — as would a pool held as a
class member (move the owner and every mesh inside it dangles), and any factory returning a pool
alongside the geometry built into it. The moved-from-pool `EXPECT` does not cover these: it fires
only while the moved-from object is still *alive*, and `EBGEOMETRY_EXPECT` compiles to `((void)0)`
in the release presets. This is not a new way for users to get lifetimes wrong; it is a pattern that
works today and stops working, with nothing in the diff to draw a reviewer's eye to it.

The fix is to point at something a move cannot relocate:

```cpp
struct PoolControl
{
  void*    m_base = nullptr;   // written by grow(); read by every descriptor
  uint64_t m_id   = 0;         // pool identity, for rebasedView's lineage check
};
```

`Pool` holds `std::unique_ptr<PoolControl> m_control` and **drops its own `m_base` field** — one
source of truth, nothing to keep in sync. Moving a `Pool` moves the `unique_ptr`; the pointee's
address is unchanged, so every descriptor keeps resolving. `grow()` writes the new base into the
control block, which is what delivers the growth-immunity this PR exists for.

Touched sites in `Pool`, all mechanical:

- `base()` — `return m_control != nullptr ? m_control->m_base : nullptr;`
- `grow()` — reads `m_control->m_base` for the `memcpy`/`deallocate`, then writes the new base back.
- `~Pool()` and move assignment — deallocate via `m_control->m_base`.
- Both constructors (the public one and the private `MirrorTag` one) — allocate the control block.
- Move constructor — moves the `unique_ptr` and needs no base fixup at all. A moved-from pool has
  `m_control == nullptr`, so `base()` returns null and the destructor frees nothing: the same
  observable behaviour as today's `a_other.m_base = nullptr`, so the existing `TestPool` move
  expectations hold unchanged.

`m_size`, `m_capacity`, `m_frozen`, `m_resource` and `m_mirrorOf` stay on the `Pool` itself —
nothing that resolves a descriptor needs them, and `rebasedView` reads them through the `const Pool&`
it is handed. `m_id` lives in the control block because a descriptor must be able to name its own
source pool for the lineage check without holding a `Pool*`.

Cost: one small heap allocation per pool (against a block that is normally megabytes), and exactly
the one indirection `m_pool->base()` would have cost. `Pool` stays movable, which it must be —
`mirror()` returns by value.

What this does **not** buy is use-after-*destroy* detection: destroying a pool frees its block, so
its descriptors dangle. That is unchanged from today's contract — the pool must outlive its meshes
either way — so it is parity, not a regression. Making it detectable would require the control
block to outlive the pool (a registry, or a deliberate leak), which is out of scope here.

### What this deletes

- `MeshT::bind()` — there is nothing to bind. **There is no "unbound" state any more**: a host mesh
  has `m_control` from its first `reserveX(pool, …)`, a device view has `m_base` from `rebasedView()`,
  and the only state with neither is a default-constructed mesh nobody reserved into (caught by an
  `EBGEOMETRY_EXPECT` in `base()`).
- **Every explicit-base overload.** Public: `getVertex/getEdge/getFace(base, i)` (mutable and const),
  `signedDistance(base, p)` (both forms), `unsignedDistance2(base, p)`, `getAllVertexCoordinates(base)`,
  `reconcile(base, …)`, `sanityCheck(base, id)`, `setInsideOutsideAlgorithm(base, alg)`, `flip(base)`.
  Protected: `reconcileFaces/reconcileEdges/reconcileVertices(base, …)`,
  `flipFaceNormals/flipEdgeNormals/flipVertexNormals(base)`,
  `DirectSignedDistance/DirectSignedDistance2(base, p)`. They exist solely to resolve mid-build
  before `bind()` was legal. With `m_control` captured, the no-argument forms are always correct, so
  `MeshT`'s accessor surface roughly halves. The `FaceT`/`EdgeT`/`VertexT` methods that take a
  `const Mesh&` are untouched — the mesh they receive always resolves, which is precisely what
  `boundView` existed to fake.
- `boundView()` in its **host** role, which is entangled with the bullet above rather than separable
  from it. The two *external* call sites do pass `a_pool.base()`
  (`MeshDistanceFunctionsImplem.hpp:446`, `ParserImplem.hpp:1861`), i.e. the pool's own current base,
  which is exactly what `m_control->m_base` already holds, so those become no-ops. But there are also
  seven *internal* uses in `DCEL_MeshImplem.hpp` (`:158,463,475,487,617,695,726`) that pass a
  parameter base — they disappear only as a consequence of deleting the explicit-base family, so the
  two deletions land together or not at all.
- The freeze-before-query rule, and with it the three-pool workaround in `Examples/MeshSDF` and
  the two-pass dance in `Parser::readIntoMesh`.
- `deepCopy(srcBase, dstPool)` → `deepCopy(dstPool)`; the source base is known.
- `freeze()` calls in the `FlatMeshSDF`/`MeshSDF` constructors.

`freeze()` survives, required **only** before `mirror()` — a rule that justifies itself ("you
cannot copy a block that might still move") rather than being a precondition on asking a question.

### What replaces the freeze rule: resolved references die at the next `reserve`

Deleting freeze-before-query deletes a structural guarantee, and it has to come back as a documented
rule. Today the ergonomic accessors require a frozen pool, and `reserve()` after `freeze()` is
forbidden (`PoolImplem.hpp:98`), so a reference into pool memory cannot outlive a base move — the
state machine makes it unreachable. Once a mesh can be queried mid-build, it becomes reachable:

```cpp
Edge& e = mesh.getEdge(3);      // resolved once: a raw address into the current block
mesh.reserveFaces(pool, n);     // may grow — grow() deallocates the old block (PoolImplem.hpp:133)
e.setFace(7);                   // write to freed memory
```

The `PODVector` is fine (it stores an offset, not an address) and the edge data is fine (`memcpy`'d
to the same offset in the new block). What is stale is the address `getEdge(3)` already computed.
The failure is intermittent — `reserve` only grows when `need > m_capacity` — and its quiet form is
worse than a crash: if the freed page is still mapped, the write lands in the abandoned copy, and
the mesh, now reading from the new block, silently reports the old value.

The rule is: **resolve, use, discard — a resolved address must not outlive the next `reserve` on
that pool.** Holding data across a `reserve` means holding it *by value*:

```cpp
Edge e = mesh.getEdge(3);       // snapshot, independent of any base
mesh.reserveFaces(pool, n);
e.setFace(7);
mesh.getEdge(3) = e;            // fresh resolution against the current base
```

The exposure is exactly the reference-returning accessors (`getVertex`/`getEdge`/`getFace`) plus
`PODVector::bind()`; everything returning by value (`signedDistance`, `getAllVertexCoordinates`) is
unaffected. Note that `PODVector`'s class documentation already states this correctly, but only for
`bind`/`PODSpan` — the `T&` returned by `at()` carries the identical restriction and says nothing.

Documented in the same PR, in four places:

1. **`Pool::reserve`'s Doxygen** — its `@details` currently frames growth as an allocation detail
   ("May grow the block"). It must say that a grow deallocates the old block and invalidates every
   previously-resolved pointer, reference and `PODSpan` into it, while leaving every `PODVector`
   valid.
2. **`PODVector`'s class-level block** — generalize the existing `bind`/`PODSpan` caveat to cover
   anything resolved, including `at()`'s return, and show the snapshot-and-write-back pattern.
3. **`MeshT`'s `getVertex`/`getEdge`/`getFace` Doxygen** — the one-liner plus a pointer to the rule.
4. **`MemoryModel.rst`** — as prominently as the freeze rule it replaces.

### The one sanctioned crossing

```cpp
EBGEOMETRY_HOST
Mesh
MeshT<T, Meta>::rebasedView(const Pool& a_pool) const noexcept
{
  EBGEOMETRY_EXPECT(m_control != nullptr);                       // not already a view
  EBGEOMETRY_EXPECT(a_pool.mirrorOf() == m_control->m_id);       // a mirror of *our* pool
  EBGEOMETRY_EXPECT(m_vertices.endByte() <= a_pool.usedBytes()); // our arrays fit inside it
  EBGEOMETRY_EXPECT(m_edges.endByte()    <= a_pool.usedBytes());
  EBGEOMETRY_EXPECT(m_faces.endByte()    <= a_pool.usedBytes());

  Mesh view = *this;

  if (a_pool.resource().isDeviceAccessible()) {
    EBGEOMETRY_EXPECT(a_pool.isFrozen());   // snapshotting a base: it must not move
    view.m_control = nullptr;               // a kernel cannot follow a control block
    view.m_base    = a_pool.base();
  }
  else {
    view.m_control = a_pool.control();      // growth-immune, exactly like the original
    view.m_base    = nullptr;
  }

  return view;
}
```

Taking the `Pool&` rather than a `void*` is what makes the checks possible. The bounds and lineage
checks catch the realistic mistakes (wrong pool, mirrored before the last `reserve`), which a bare
base pointer cannot detect at all.

**One function, not two, and `isDeviceAccessible()` is the discriminator rather than an assertion.**
Rebasing onto a host-to-host mirror must be supported: `Pool::mirror` explicitly offers it ("and,
host-to-host, an exact independent copy", `Pool.hpp:178-179`), and `Tests/TestPoolRebase.cpp` is
built on it precisely because it exercises the rebase invariant *without a GPU* — the only way that
invariant executes in CI at all, since the CI runners have no device. Asserting device-accessibility
would have made a `MeshT`-level version of that test impossible to write. Making the predicate select
the branch instead means the caller states their intent by which pool they hand over, with no second
function to choose between: `Managed`/`Mapped` are device-accessible and correctly take the snapshot
branch, `Pinned` and `Host` take the control-block branch.

The host branch is the reason `base()`'s assertions above can be exact. Following the target's
control block rather than snapshotting its base means a host rebase leaves `m_control` non-null, so
a null `m_control` is unambiguously a device view — no residency flag needed to tell the two apart.

`EXPECT(isFrozen())` on the device branch never actually binds: the lineage check already restricts
the target to a mirror of our pool, `mirror()` returns frozen pools, there is no unfreeze, and
`grow()` refuses non-host-accessible resources outright (`PoolImplem.hpp:119`). It is kept as a
cheap guard on the one place a raw base is captured, not because a live path can violate it. The
host branch needs no such guard at all — that is what the control block buys.

`rebasedView` is `EBGEOMETRY_HOST` — `Pool` is host-only, and building a kernel argument is a host
activity. The old `HOST_DEVICE` on `boundView` was justified by a device-side rebase that nothing
uses.

### Supporting changes

- **`Pool`**: replace `void* m_base` with `std::unique_ptr<PoolControl> m_control` (see above), and
  add a `control()` accessor returning `const PoolControl*`. `m_id` (monotonic, assigned at
  construction) lives *in the control block*; `uint64_t m_mirrorOf` stays on the `Pool`, with
  `id()` / `mirrorOf()` accessors. The id counter must be an `inline std::atomic<uint64_t>` — the
  library is header-only and pools may be constructed on several threads.
- **`mirrorOf` names the root, not the immediate source**: `mirror()` sets
  `m_mirrorOf = a_src.m_mirrorOf != 0 ? a_src.m_mirrorOf : a_src.id()`, and 0 means "not a mirror".
  Without this, a chain (host → pinned staging → device, which is most of why a `Pinned` resource
  exists) leaves the final pool naming the staging pool, so `rebasedView`'s lineage check fails on a
  correct sequence — the block is a faithful copy and every offset resolves, only the bookkeeping
  disagrees. Carrying the root forward makes the check transitive for chains of any length while
  still rejecting a genuinely unrelated pool.
- **`PODVector`**: add `endByte()` = `m_offset + m_capacity * sizeof(T)` for the bounds checks.
  Deliberately capacity, not size: it bounds the *reserved* region against the mirrored block.
- **`MeshT`**: `m_control` is captured (from `a_pool.control()`) on the first
  `reserveVertices/Edges/Faces(pool, …)`, with an `EBGEOMETRY_EXPECT` that later reserves pass the
  same pool.
- **Parsers**: `readInto*` no longer freeze; the multi-file overloads become plain loops.
- **SDF wrappers**: `FlatMeshSDF`/`MeshSDF` stop freezing and binding, matching what
  `TriMeshSDF`'s mesh constructor already does.
- **`Examples/MeshSDF`**: collapse three pools into one.

### Pattern reuse

`PackedBVH` needs the same two fields (`m_control`, `m_base`), the same `base()`, and its own
`rebasedView`. Duplicate them rather than introducing a CRTP mixin — it is ~15 lines, and a base
class complicates the trivial-copyability `static_assert`s for no real saving. Document the
convention once in `PORTING.md`.

### Risks

- **Pool lifetime.** Moves are safe (the control block does not relocate), but *destroying* a pool
  still invalidates every descriptor into it, since the block dies with it. Unchanged from today,
  and not detectable without a registry outliving the pool.
- **Resolved references.** See the section above: the freeze state machine used to make a live
  reference across a base move structurally impossible, and now only documentation and review do.
- **Query-during-build is not thread-safe.** With freeze gone as a precondition, a query on one
  thread races a `reserve()` on another against the same pool — a data race on the control block's
  base, and possibly a `grow()` under the reader's feet. Today that is structurally impossible.
  Needs a sentence in `MemoryModel.rst`; the library is used from OpenMP codes.
- **Weakened invariant.** #140 made "mirror a host descriptor, dereference on device" structurally
  impossible; it becomes possible-but-asserted. The assertions are exact rather than heuristic (see
  "The rule"), and every realistic error path is caught at the point of the mistake rather than as a
  garbage kernel result — but `EBGEOMETRY_EXPECT` compiles to `((void)0)` in the release presets, so
  this is a debug-time guarantee, not a type-level one. Recovering the type-level version means a
  distinct view type, which is not "two types and an API break" as the rejected-alternatives list
  above says, but two types *plus* templating every `FaceT`/`EdgeT`/`VertexT` method that takes a
  `const Mesh&`. That cost is the actual reason to accept this trade, and it should be recorded as
  such.

### Docs

`MemoryModel.rst` needs its central framing rewritten: build/freeze/query becomes build-and-query,
with freeze as a mirror precondition. It gains, in freeze's place, the resolved-reference rule and
the note that querying during a build is not thread-safe against a concurrent `reserve`.
`ImplemDCEL.rst`'s memory-model section loses the explicit-base/bound accessor split and the
`boundView` note added in #140.

Header Doxygen is not optional here and is listed with the four documentation sites in the
resolved-reference section above: `Pool::reserve`, `PODVector`'s class block, and `MeshT`'s
reference-returning accessors. `Pool`'s own file-level block also describes the two-phase contract
in its current form and needs the same rewrite as `MemoryModel.rst`.

`PORTING.md` is part of this PR too, not a follow-up: its "one rule" section shows
`mesh->boundView(devicePool.base())` as *the* sanctioned crossing, and its "Porting a class" recipe
and DCEL row both describe the accessor-pair convention this PR removes. It is accurate today, so it
must change in the same commit that makes it wrong.

## PR 2 — the BVH port

### Storage policies

Keep the mechanism, but reduce it to two POD policies. `SharedPtrStorage` is **purged**.

| Policy | Indirection | Dedup | Needs a canonical owner array | Crosses to device |
|---|---|---|---|---|
| `ValueStorage` | none | no | — | yes |
| `IndexStorage` *(new)* | index | yes | yes | yes |

`IndexStorage<P>` is `StorageType = uint32_t`, resolved as `static_cast<const P*>(base)[idx]`. It
reproduces what `SharedPtrStorage` was actually wanted for — one primitive set shared by 1000
translated copies, edited in one place — at 4 bytes instead of 16 plus a control block. What it
cannot reproduce is **automatic lifetime**: the owning pool must outlive every copy, enforced by
convention rather than refcount.

`get()` gains a base parameter: `get(const StorageType&, const void* a_base)`. `Value` ignores it.
Three call sites (`MeshDistanceFunctionsImplem.hpp:522,559`, `BVHImplem.hpp:1635`).

Purging `SharedPtrStorage` is what keeps this clean: both remaining policies are trivially
copyable, so `m_primitives` is a `PODVector` in every instantiation and `PackedBVH` needs no
`std::vector` fallback backend. `TreeBVH` keeps its own `shared_ptr` storage — it is the host-only
builder and is not policy-parameterised — so packing simply dereferences those into values (which
`ValueStorage::appendTreeLeaf` already does: `a_dst.push_back(*p)`).

What the purge breaks, all of which must be updated in the same PR:

- `MeshSDF::getClosestFaces` returns `std::vector<std::pair<std::shared_ptr<const FaceT>, T>>`
  (`MeshDistanceFunctionsImplem.hpp:309`). It should return `std::vector<std::pair<uint32_t, T>>` —
  face indices, not copies. More useful than the pointers were, and it avoids copying ~80 B faces.
- `MeshDistanceFunctions.hpp:144` ("its PackedBVH always uses BVH::SharedPtrStorage<Face>") and
  `Parser.hpp:260` — doc rewrites.
- `SharedPtrStorage::appendAliased`'s aliasing-constructor trick and the comment at
  `BVHImplem.hpp:506` disappear with it.
- `PrimitiveList<P>` (`std::vector<std::shared_ptr<const P>>`) survives as `TreeBVH`'s own storage;
  it simply no longer flows into `PackedBVH`.

**Open wrinkle — filling `IndexStorage` from a `TreeBVH`.** A tree holds `shared_ptr<const P>` with
no notion of the owner's array index, so `appendTreeLeaf` cannot produce indices from it. Options:
have `MeshSDF` build the tree over face indices directly (`TreeBVH<T, uint32_t, AABB, K>` with
caller-supplied BVs — but that heap-allocates a `shared_ptr<const uint32_t>` per primitive), or
give the packing path an explicit primitive→index mapping. `ValueStorage` has no such problem, so
this only gates `IndexStorage`'s build path and can be settled inside PR 2.

**Defaults**: `MeshSDF` → `ValueStorage` (matches today's semantics, minus ~one heap allocation
per face, and gives a contiguous leaf scan because packing reorders primitives into leaf order).
`IndexStorage` is the opt-in for large meshes where the ~80 B/face duplication is the binding
constraint, trading locality for memory. `TriMeshSDF` stays on `ValueStorage`.

### Pool-backed storage

All three arrays become `PODVector`: `m_linearNodes`, `m_childAabbSoA`, `m_primitives`. `Node` and
`ChildAABBSoA` are already trivially copyable (`AABBT<T>` + `uint32_t`s + `std::array<uint32_t,K>`),
so only the containers change.

Every construction entry point gains a `Pool&`: `pack(Pool&)`, `packWith(Pool&, converter)`, the
direct `primsAndBVs` constructors, and the protected constructor `PointCloudBVH` uses. `MeshSDF`
already takes a `Pool&`, so its signature is unchanged and the mesh and its BVH land in one pool —
one `mirror()`, one base, one `rebasedView`.

`TreeBVH` stays host-only and unchanged. It is the builder; static geometry builds on the host.

### Traversal

`pruneTraverse` is currently 513 lines containing **seven** copies of the same branch-and-bound
loop — six `if constexpr` SIMD specialisations plus a scalar fallback that has no implementation of
its own (it builds four `std::function`s and delegates to `traverse()`, which runs on a heap
`std::vector` stack). The GPU can only ever take that fallback.

Factor the loop **once**, parameterised on a child-distance evaluator (`ChildDistSIMD<T,K>` /
`ChildDistScalar<T,K>`), so each ISA block shrinks to the handful of intrinsics it actually
contributes and the device path is an eighth evaluator rather than an eighth transcription. Give
the scalar path a real implementation: fixed stack, hand-rolled insertion sort over the ≤K children
(`std::sort` and `std::clamp` both drag in host-only libstdc++ helpers under hardened mode).

**Stack depth** becomes a parameter differing by entry point: 256 on host, 64 on device.
`StackEntry` is 16 B at `double`, so today's `[256]` is 4 KB/thread of local memory;
`log_K(N)·K` says 64 covers a million primitives at K=4.

**Callables on device**: we compile with `--expt-relaxed-constexpr` but not `--extended-lambda`, so
device callers must pass functors, not lambdas. Keep the templated API for that, and additionally
ship the two common queries (signed distance, closest point) as ready-made device entry points so
ordinary users never write a callback.

`refit()` stays host-only for now — it is a reverse sweep over `std::vector<BV>` scratch, and
nothing yet needs moving geometry to stay resident on the device.

### Sequence within PR 2

1. Traversal refactor + real scalar path. Host-only, no API change, fully covered by `TestBVH`.
   Also a CPU speedup for any (T,K) with no compiled ISA path.
2. `IndexStorage` + the `get(stored, base)` signature.
3. Arrays onto `Pool`/`PODVector`; `Pool&` on the constructors; `m_control`/`m_base`/`rebasedView`.
4. `[gpu]` case in `TestBVH.cpp`; add `TestBVH` to `EBGEOMETRY_GPU_TESTS`.
5. `TriMeshSDF` on device, then `MeshSDF`.

If (1) makes the PR unwieldy it can be pulled out as a small host-only PR of its own — it is
independent of everything else here.

### Open questions

- `MeshSDF`'s default policy. `Value` matches today's semantics and gives a contiguous leaf scan;
  `Index` duplicates nothing at all, since the faces already live in the same pool. On the merits
  it is a locality-vs-memory judgement that depends on mesh size, but `Value` wins for PR 2 on a
  practical ground: its build path already works, while `Index`'s needs the primitive→index mapping
  described above. `Index` can follow immediately after without touching any of the pool or
  traversal work.
- Whether the `getClosestFaces` signature change (to `std::vector<std::pair<uint32_t, T>>`) should
  carry an index into the mesh's face array or into the BVH's own primitive array. The former is
  more useful to a caller; the latter is what the traversal naturally produces.

## Verification

Both PRs: full debug suite, plus `cmake --preset cuda -DCMAKE_CUDA_ARCHITECTURES=86` and
`ctest -L gpu-device` on the local RTX 3080 Ti. CI compiles both backends but has no GPU, so a
green GPU lane means "it compiles" and nothing more.

PR 1 specifically needs new tests that would have been impossible before:

- Query an object, force the pool to grow, query again, require the same answer.
- Move the owning `Pool` — including through a `std::vector<Pool>` reallocation, which destroys the
  moved-from object rather than merely emptying it — and require every descriptor to keep resolving.
  This is the control block's whole reason for existing, and the `Pool*` design fails it.
- Cover the *reference* path, not just the value path. The grow test above passes green while the
  dangling-reference hazard is wide open, so add a case that snapshots by value across a grow and
  writes back, i.e. the pattern the docs will prescribe.
- A `deepCopy` into the *same* pool, which currently reserves three times against a cached
  `a_srcBase` (`DCEL_MeshImplem.hpp:66-95`) and is a latent use-after-free that this PR removes by
  construction.
- A `MeshT`-level host-to-host `rebasedView`: build a mesh, `mirror()` its pool into a second host
  pool, rebase, and require identical query results against both — `TestPoolRebase.cpp`'s proof
  applied to real geometry rather than a synthetic `Scene`. This runs in CI, which has no GPU, and
  is the reason `isDeviceAccessible()` is a discriminator rather than an assertion.
