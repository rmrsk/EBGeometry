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
Pool* m_pool = nullptr;   // host-only. null in a device view.
void* m_base = nullptr;   // resolved base. authoritative on device.

EBGEOMETRY_HOST_DEVICE const void* base() const noexcept
{
#if defined(EBGEOMETRY_DEVICE_COMPILE)
  EBGEOMETRY_EXPECT(m_pool == nullptr);   // a host descriptor reached a kernel without deviceView
  return m_base;
#else
  EBGEOMETRY_EXPECT(m_pool == nullptr || m_pool->base() != nullptr);   // moved-from pool
  return m_pool != nullptr ? m_pool->base() : m_base;
#endif
}
```

The two are not redundant: `m_base` points *into the data block*, which `mirror()` copies, so it
has a device counterpart and can be rebased. `m_pool` points at *host bookkeeping* that is never
mirrored and has no device counterpart. Host resolution goes through the pool and is therefore
immune to growth; device resolution uses the rebased base.

The type stays trivially copyable (two raw pointers plus `PODVector`s), so every existing
`static_assert` and `Meta` requirement holds unchanged.

### What this deletes

- `MeshT::bind()` — there is nothing to bind. **There is no "unbound" state any more**: a host mesh
  has `m_pool` from its first `reserveX(pool, …)`, a device view has `m_base` from `deviceView()`,
  and the only state with neither is a default-constructed mesh nobody reserved into (caught by an
  `EBGEOMETRY_EXPECT` in `base()`).
- **Every explicit-base overload.** `getVertex(base, i)`, `signedDistance(base, p)`,
  `unsignedDistance2(base, p)`, `getAllVertexCoordinates(base)`, `reconcile(base, …)`,
  `reconcileFaces(base)`, `reconcileVertices(base, …)` exist solely to resolve mid-build before
  `bind()` was legal. With `m_pool` captured, the no-argument forms are always correct, so
  `MeshT`'s accessor surface roughly halves. The `FaceT`/`EdgeT`/`VertexT` methods that take a
  `const Mesh&` are untouched — the mesh they receive always resolves, which is precisely what
  `boundView` existed to fake.
- `boundView()` in its **host** role. Every host call site passes `a_pool.base()`
  (`MeshDistanceFunctionsImplem.hpp:445`, `ParserImplem.hpp:1860`), i.e. the pool's own current
  base, which is exactly what `m_pool->base()` already returns. Those calls become no-ops.
- The freeze-before-query rule, and with it the three-pool workaround in `Examples/MeshSDF` and
  the two-pass dance in `Parser::readIntoMesh`.
- `deepCopy(srcBase, dstPool)` → `deepCopy(dstPool)`; the source base is known.
- `freeze()` calls in the `FlatMeshSDF`/`MeshSDF` constructors.

`freeze()` survives, required **only** before `mirror()` — a rule that justifies itself ("you
cannot copy a block that might still move") rather than being a precondition on asking a question.

### The one sanctioned crossing

```cpp
EBGEOMETRY_HOST
Mesh
MeshT<T, Meta>::deviceView(const Pool& a_devicePool) const noexcept
{
  EBGEOMETRY_EXPECT(a_devicePool.resource().isDeviceAccessible());    // reachable from a kernel
  EBGEOMETRY_EXPECT(a_devicePool.isFrozen());                         // a real, sealed mirror
  EBGEOMETRY_EXPECT(a_devicePool.mirrorOf() == m_pool->id());         // a mirror of *our* pool
  EBGEOMETRY_EXPECT(m_vertices.endByte() <= a_devicePool.usedBytes());// our arrays fit inside it
  EBGEOMETRY_EXPECT(m_edges.endByte()    <= a_devicePool.usedBytes());
  EBGEOMETRY_EXPECT(m_faces.endByte()    <= a_devicePool.usedBytes());

  Mesh view   = *this;
  view.m_pool = nullptr;
  view.m_base = a_devicePool.base();
  return view;
}
```

Taking the `Pool&` rather than a `void*` is what makes the checks possible. `isDeviceAccessible()`
already exists on all five resources and is the correct predicate — `Managed`/`Mapped` are both
host- and device-accessible, `Pinned` is host-accessible but **not** device-reachable. The bounds
and lineage checks catch the realistic mistakes (wrong pool, mirrored before the last `reserve`),
which a bare base pointer cannot detect at all.

`deviceView` is `EBGEOMETRY_HOST` — `Pool` is host-only, and building a kernel argument is a host
activity. The old `HOST_DEVICE` on `boundView` was justified by a device-side rebase that nothing
uses.

### Supporting changes

- **`Pool`**: add `uint64_t m_id` (monotonic, assigned at construction) and `uint64_t m_mirrorOf`
  (set by `mirror()` to the source's id, 0 otherwise), with `id()` / `mirrorOf()` accessors.
- **`PODVector`**: add `endByte()` = `m_offset + m_capacity * sizeof(T)` for the bounds checks.
- **`MeshT`**: `m_pool` is captured on the first `reserveVertices/Edges/Faces(pool, …)`, with an
  `EBGEOMETRY_EXPECT` that later reserves pass the same pool.
- **Parsers**: `readInto*` no longer freeze; the multi-file overloads become plain loops.
- **SDF wrappers**: `FlatMeshSDF`/`MeshSDF` stop freezing and binding, matching what
  `TriMeshSDF`'s mesh constructor already does.
- **`Examples/MeshSDF`**: collapse three pools into one.

### Pattern reuse

`PackedBVH` needs the same two fields, accessor, and `deviceView`. Duplicate them rather than
introducing a CRTP mixin — it is ~15 lines, and a base class complicates the trivial-copyability
`static_assert`s for no real saving. Document the convention once in `PORTING.md`.

### Risks

- **`Pool` is movable** (`mirror()` returns by value), so moving one leaves descriptors pointing at
  a moved-from object. A moved-from pool has `m_base == nullptr`, caught by the `EXPECT` in
  `base()`.
- **Weakened invariant.** #140 made "mirror a host descriptor, dereference on device" structurally
  impossible; it becomes possible-but-asserted. Every realistic error path is now caught on the
  *host* at the point of the mistake, which is a more useful place than a garbage kernel result,
  but it is a debug-time guarantee rather than a type-level one.

### Docs

`MemoryModel.rst` needs its central framing rewritten: build/freeze/query becomes build-and-query,
with freeze as a mirror precondition. `ImplemDCEL.rst`'s memory-model section loses the
explicit-base/bound accessor split and the `boundView` note added in #140.

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
one `mirror()`, one base, one `deviceView`.

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
3. Arrays onto `Pool`/`PODVector`; `Pool&` on the constructors; `m_pool`/`m_base`/`deviceView`.
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

PR 1 specifically needs a new test that would have been impossible before: query an object, force
the pool to grow, query again, and require the same answer.
