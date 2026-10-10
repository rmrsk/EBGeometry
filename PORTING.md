# Porting EBGeometry to GPUs

Working overview of the GPU port: the one design rule everything follows, what a first attempt got
wrong, where the port stands, what remains, and the design decisions made along the way with the
evidence behind them. This file is the authoritative status document. It replaces `PLAN.md` and
`BVH_PORT_SUMMARY.md`, which carried the detail of the BVH port and were folded in here and deleted
(audit decision D14). [Issue #115](https://github.com/rmrsk/EBGeometry/issues/115) still holds the
original design discussion and is worth reading for the *rationale* behind individual decisions,
but its tier checklist describes the abandoned first attempt and is **stale**. Where the two
disagree, this file wins.

Until the tape starts, [AUDIT.md](AUDIT.md) carries the remaining sequence (its Phase 3) and the
inputs to the tape's design (its Appendix A). Audit item 26 moves what is still live from it into
this file and the tape design document, and deletes it.

Status, through audit item 24: every class meant to run on a device is ported, the pre-tape audit's
Phases 0–2 are done, and the shape conventions (audit item 24) are settled. One thing remains before
the tape: a machine that can run kernels (see "Before the tape").

## The one rule

> **Store an offset, not a pointer. The base is supplied by the caller.**

Every other convention in the port follows from this. A pointer that is valid in host address space
is meaningless once the same bytes are copied to a device, so no structure that has to cross may
contain one. Structures instead store byte offsets relative to a base address passed in separately,
which makes the identical bit pattern resolve correctly against a host base *and* a device base —
with no pointer-patching pass after the copy.

The practical consequence is that a portable structure has **at most one** address-space-specific
field, its `PoolLocation`, and it is set at the moment of crossing rather than being a property of
the object. `rebasedView(pool)` is the sanctioned way to produce a copy that resolves in a different
address space:

```cpp
hostPool.freeze();
EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(hostPool, EBGeometry::deviceMemoryResource());
const auto       deviceSDF  = hostSDF.rebasedView(devicePool);     // rebase on the host …
myKernel<<<blocks, threads>>>(deviceSDF, …);                      // … then copy by value
```

Rebase, then copy — never copy a host-resident value and try to repair it afterwards.

A host-resident object does *not* hold a base at all: its `PoolLocation` points to its `Pool`'s
control block and re-reads the base through it on every access. That is what makes it immune to a
`reserve()` that grows and moves the block. A rebased view holds the base address itself, so it
needs no control block on the device.

## Why the first attempt was rewound

An earlier port ran from PR #121 to #129 as "Tiers 0–9" and was abandoned; none of it is on `dev`,
whose history goes straight from #120 to #130. It was not a failure of the *evaluation* design — the
tape (see below) worked and was validated — but of memory placement, which it treated as a final
integration step rather than a foundation.

What it built:

* `Tape<T>` held the compiled clause list, the per-op SoA parameter arrays, scalar parameters and
  four BVH arrays — all as `std::vector` members.
* `TapeView<T>` was a struct of **raw pointers** into those vectors (`const TapeClause* clauses`,
  `const T* scalarParams`, `const uint32_t* bvhChildren`, …), handed to the interpreter.
* `DeviceTape<T>` (added at Tier 7, *after* the structures were designed) uploaded a tape by making
  one `GPU::memAlloc`, copying each vector into it at 256-byte-padded offsets, and then building a
  **second** `TapeView` whose pointers were device addresses.

Three things went wrong, and each has a direct counterpart in the current design:

| First attempt | Consequence | Now |
|---|---|---|
| View held raw pointers | The same view value could never be valid on both sides; every pointer had to be recomputed after upload | `PODVector` stores an offset, so one value resolves against either base |
| `DeviceTape` hand-rolled a pool — `DeviceTapeAlign = 256`, `padUp()`, `paddedBytes()`, `uploadArray()` | A single-purpose allocator, reinvented per structure, reusable by nothing else | `MemoryResource` + `Pool` are that same idea factored out; `PoolBaseAlign` is literally the same 256 bytes |
| Upload enumerated every array by hand (mitigated by driving it from the X-macro registry) | Adding an array to `Tape` meant remembering to add it to `DeviceTape` | `Pool::mirror()` is one `memcpy` of the whole block; adding an array costs nothing |

Underneath all three: the data structures were designed host-first around `std::vector`, and device
residency was bolted on later. Everything the upload path did — pooling, padding, offsetting — was
work created by that ordering, and it had to be redone for every new structure. The restart
inverted the order deliberately: the memory foundation landed **first**, as PR #130, before any
geometry class was touched.

This is also why the working agreement forbids *host/device data duality*: one flat,
trivially-copyable POD type used on both sides, never a `std::vector` host representation paired
with a POD device mirror. `std::vector` is acceptable only in host-only build steps that never
cross, and those are marked `EBGEOMETRY_HOST`.

### What survived

Not everything was discarded. The backend portability layer — `EBGeometry_GPU.hpp` (the
`EBGEOMETRY_HOST` / `EBGEOMETRY_HOST_DEVICE` / `EBGEOMETRY_GLOBAL` decorations, device-safe
`EBGEOMETRY_EXPECT`) and `EBGeometry_GPURuntime.hpp` (the backend-neutral `EBGeometry::GPU::*`
wrappers over the CUDA/HIP runtime) — was carried forward into #130, extended only with the managed,
pinned and mapped allocation wrappers the new `MemoryResource` types need.

The *evaluation* design also stands and is expected to return with the tape: a linear-SSA tape in
the MPR/libfive style, one trait per primitive as the single source of truth for its formula, a
generated opcode registry, smooth-blend operators as data rather than `std::function`, and a fixed
`uint32` winning-primitive index as the only thing transported per SSA slot. Those decisions and
their reasoning are recorded in the comments on issue #115 and summarised in `AUDIT.md`'s
Appendix A; they do not need re-deriving.

## The current foundation

Three layers, documented in full on the
[Memory model](https://rmrsk.github.io/EBGeometry/MemoryModel.html) page:

* **`MemoryResource`** — a runtime-virtual placement policy deciding *where* bytes live. Host,
  Device, Managed, Pinned and Mapped resources exist; the latter four require a CUDA/HIP build.
  Allocation is always a host-side operation, and every block is aligned to `PoolBaseAlign` (256 B).
* **`Pool`** — one contiguous block from a resource. `reserve()` bump-allocates and may grow,
  which reallocates and frees the old block; the current base is published through a heap-resident
  `PoolControl`, so pool-resident objects see the move and already-resolved addresses do not.
  `freeze()` seals the pool and exists solely as the precondition for `mirror()`, which copies a
  whole frozen block into another resource — the host-to-device upload, one `memcpy`, no patching.
  Move-only (moving a `Pool` does not disturb its control block), host-only; never pass a `Pool` to
  a kernel.
* **`PODVector` / `PODSpan`** — array descriptors holding a byte offset plus size and capacity.
  Trivially copyable, resolved against a base supplied per call (`at(base, i)`) or bound once
  (`bind(base)`).

Classes built on this expose **one** accessor per operation. They attach to a `Pool` on their first
`reserve` and resolve through its control block from then on, so there is no bound/unbound
distinction, no freeze-before-query rule, and no base parameter threaded through the API.
`PoolLocation` owns the location logic (attach, lineage checks, rebasing), and `rebasedView(pool)`
is the single crossing point. A pool-resident object stored inside another one's pool (a mesh SDF as
a BVH-union primitive) is evaluated through `relocatedTo()`, which applies the outer object's
location to a copy of it.

Two rules follow from this and must be documented on anything that adopts it: a resolved address
(a reference, a `PODSpan`) must not outlive the next `reserve()` on its pool, and the pool must
outlive every object reserved from it.

## What is ported

Device-callable, trivially copyable, and covered by a `[gpu]`-tagged test that evaluates it on many
queries in a kernel and compares against the host:

| Component | Headers | PRs |
|---|---|---|
| Memory foundation | `MemoryResource`, `Pool`, `PODVector`, `PoolLocation` | #130, #143, #153 |
| Vectors | `EBGeometry_Vec.hpp` | #131 |
| Bounding volumes | `AABBT` in `EBGeometry_BoundingVolumes.hpp` (`SphereT` retired) | #132, #133, #164 |
| SoA/AoSoA leaves | `PointSoA`, `PointAoSoA`, `TriangleSoA`, `TriangleAoSoA` | #134 |
| DCEL | `VertexT`, `EdgeT`, `FaceT`, `EdgeIteratorT`, `MeshT` | #137–#140 |
| BVH traversal and storage | `PackedBVH` (`Node`, `ChildAABBSoA`, `pruneTraverse`, the template `traverse`) | #145, #146, #153, #154, #161 |
| Point-cloud queries | `PointCloudBVH` (holds a `PackedBVH`) and `PointCloudHashGrid` (pool-backed grid); one `PointCloud::Hit`, a shared `PointCloud::KBest`, device-callable brute-force references. Both builds stay on the host | #147, #162 |
| Mesh SDFs | `FlatMeshSDF`, `MeshSDF`, `TriMeshSDF`; `getClosestFace` returns a `uint32_t` face id | #148, #149, #156 |
| Analytic shapes | The twelve classes in `EBGeometry_AnalyticDistanceFunctions.hpp` (constructors stay on the host) | #151 |
| BVH unions | `BVHUnion`, `BVHSmoothUnion` over one primitive type; `SmoothMinOp`/`SmoothMaxOp`/`ExpMinOp`/`ExpMaxOp` | #152, #164 |
| Function queries | `approximateBoundingVolumeOctree` (an explicit-stack subdivision with no stored tree) and `normal`, in `EBGeometry_FunctionQueries.hpp`; device-callable when the function is | #164 |

The AMReX integrations evaluate the mesh SDFs and the BVH unions inside AMReX's own GPU kernels
(#150, #165); see "Examples and integrations" below.

## What is not

| Component | Status |
|---|---|
| `TreeBVH`, the partitioners, `pack`/`packWith`, `PackedBVH`'s builder constructors | Host-only **by design**: they are the builders, and static geometry builds on the host. Not a gap. |
| Parsers (`PolygonSoup`, `Soup`), `Random`, `SimpleTimer` | Host-only by design. `Random` and `SimpleTimer` serve the examples and are not included by `EBGeometry.hpp` |
| `Triangle<T>` (AoS) | Host-side build input for `TriangleAoSoA`; not ported, and nothing on a device needs it |
| `SFC`, point-cloud builds, `MeshT::reconcile()` | Host-only. A device build (Morton codes and a radix sort) would be needed only to rebuild on a device; see "After the tape" |
| `ImplicitFunction`, `CSG`, `Transform` | The original virtual-`value()` design, now fed by user-written implicit functions only. The tape replaces it (audit decisions D4 and D5) |
| Unions of different primitive types | Need runtime dispatch, which is the tape. The old `CSGUnion` example (a mesh plus a sphere) and the two `Shapes` integrations are tape acceptance tests until then, in `Tests/TapeAcceptance/` (D13) |

## Roadmap

The governing rule was that **every existing class is ported before the tape is started**, and that
rule is now met. The order the work was done in:

> BVH (#145, #146) → point-cloud composition (#147) → mesh SDFs (#148, #149) → analytic shapes
> (#151) → single-type BVH unions (#152) → **the pre-tape audit** (`AUDIT.md`, Phases 0–2, #153–#165)
> → shape conventions (audit item 24) → **the tape** (audit item 25).

The audit was run after the BVH unions rather than after the point clouds, the loose ends and the
integrations: its findings reshaped those steps, including several classes to retire rather than
port (`Octree::Node`, `SignedDistanceFunction`, `SphereT`, the SYCL and OpenACC branches; D2).

One consequence of porting the unions before the tape: `BVHUnion`/`BVHSmoothUnion` came back
restricted to one primitive type. A union over primitives of *different* concrete types has no way
to pick each primitive's formula on a device without some form of runtime dispatch -- which is what
the tape is -- so until then there is no such union at all; homogeneous ones need no dispatch and run
on a device. No interim dispatch mechanism is to be built in the meantime, since it would be a
second tape.

### Before the tape

**Shape conventions (audit item 24, D10): done.** The tape freezes each opcode's parameter layout,
so the shape API was settled first:

* every constructor takes the shape's position before its sizes, so `RoundedBoxSDF` and
  `RoundedCylinderSDF` gained a center;
* a built-in axis is along y, so the torus ring lies in the xz-plane and the cones open along −y;
* a size is the finished shape's, so `RoundedBoxSDF` takes the box it fits;
* every shape has a host-device `computeBoundingVolume()`, unbounded only along unbounded
  directions, which a heterogeneous union in the tape needs (CSG-7);
* a static `distanceQuality` (`Exact`, `Bound` or `NotADistance`) on every shape, mesh SDF and
  union, read by `distanceQualityOf<P>`; a BVH union derives its own from its primitive's. The
  tape's shape trait can check the same member;
* a BVH union keeps primitives with unbounded boxes out of its BVH and scans them every query,
  which a heterogeneous union in the tape (a ground plane among meshes, say) will need as well.

**A toolkit that can run kernels.** There is still no CUDA or ROCm runtime on the development
machine, so no `[gpu]` case has ever run on a real device. What exists instead covers most of it:

* Every host build runs the `[gpu]` cases in emulation (host memory that reports itself
  device-accessible, and a host loop in place of the kernel), so a device test that fails for any
  reason other than the device compile or the launch fails in the ordinary test suite.
* CI's required `GPU-HIP` job compiles every device test for a real AMD target with clang, with no
  GPU present, and the advisory `GPU-CUDA` job does the same with nvcc. That catches missing
  annotations and calls that are not device-callable.
* `Scripts/CheckDeviceMath.py` rejects the `std::` calls nvcc refuses without
  `--expt-relaxed-constexpr`.

What none of them do is execute a kernel. That needs a machine with a toolkit and a GPU, and it
should be found before the tape adds its interpreter, the largest piece of device code yet.

### The tape (audit item 25)

The linear-SSA clause list and interpreter that replaces virtual dispatch, built on
`Pool`/`PODVector` from the start rather than on `std::vector` with an upload path bolted on. Its
first steps, as the audit fixed them: the layout/handle split (D1 step two), a value-type
expression builder with a host evaluator (D4), and the retirement of `ImplicitFunction` and the
virtual transform/CSG layer **in the same change**, with their test expectations carried over to the
builder so there is no gap (D5). The tape acceptance tests in `Tests/TapeAcceptance/` must then
compile against it and produce the geometry their README describes.

**The tag/opcode registry is defined with the tape.** It was once to be defined while porting the
mesh SDFs, so they and the analytic layer shared one scheme, but only the tape consumes it;
defining it with the tape avoids designing it before its one consumer exists.

Under the tape a union's primitive is a clause id, so its BVH is a `PackedBVH<T, uint32_t, K>`: four
bytes per primitive, no indirection, buildable through every construction path, and already tested
against a BVH over the primitives themselves. "No storage policy" below has the evidence.

### After the tape

**A batched traversal path.** The device traversal is correctness-first, and is not a tuned GPU
kernel. It is the textbook formulation: one query point per thread, each with a private stack,
every lane descending its own path. That is divergence-bound by construction — the warp executes
the union of 32 different descents and retires with the slowest lane — and the private stack is
local memory, touched on every push and pop. `ChildAABBSoA` is also laid out for the wrong axis
here: it exists so one *thread* can load K children into one SIMD register, which is a CPU idea and
buys nothing when a lane reads all K serially. None of that is measured. It is recorded so the
layout is not mistaken for a finished design, and so nobody benchmarks it and draws a conclusion
about the library's GPU ceiling. The agreed shape of the fix:

* **One API, two implementations — not a GPU-special function.** Add a batched entry point,
  `pruneTraverseBatch(PODSpan<const Vec3T<T>> points, PODSpan<State> states, ...)`, implemented
  twice: on host as a loop over queries using the existing SIMD child test, on device as a mapping
  of queries onto threads or warps. Batching earns its keep on the host too (better node-cache
  reuse), so the host exercises the batched API constantly and it cannot rot. The single-query
  `pruneTraverse` stays for callers who want it.
* **Share the primitives, duplicate the loop.** `computeChildDistances2()`, the
  descending-distance child ordering, and the `State`/`LeafEvaluator`/`PruneDistSquared` contract
  stay common. The loop itself forks. Factoring on the child-distance evaluator was right for *ISA*
  variation — that is all the seven pre-port copies differed by — but a batched device traversal
  changes the parallelism axis, not the instruction: what a thread is, who owns the stack, where
  queries come from. One loop serving both would be shaped for neither.
* **Cheapest first, and measure before choosing.** Morton-sorting queries before launch needs no
  kernel change at all and attacks divergence and locality together; then warp-cooperative
  traversal (one warp per query, K children one per lane, which is what would finally make the SoA
  layout pay off on a device), stackless traversal, and persistent threads with a work queue.
  Measure a *representative* workload: the real consumers (AMReX/Chombo EB generation) evaluate
  points cell by cell over a grid, which is already close to Morton order, so a random-point
  benchmark would overstate divergence badly and could justify a rebuild on false evidence.
* **Why after the tape and not before it.** The tape changes what a leaf evaluator is, so a batched
  traversal built earlier gets partly rebuilt anyway.

Nothing needs doing now to keep this open: `pruneTraverse`'s contract already says nothing about
threads. The one thing worth adding alongside the first `[gpu]` benchmark is a coherent-query case.

**Device builds.** A device-side `PointCloudBVH` build (Morton codes and a radix sort) and its
`PointCloudHashGrid` counterpart; both already query on a device. Worth doing only with a consumer
that rebuilds geometry on the device.

**The DCEL reconcile chain on a device.** Deferred deliberately. `MeshT::reconcile()`'s only
production caller is the parser's soup-to-mesh conversion, which is host-only by design, so a device
`reconcile()` would have no device caller. Its hard part (a device-resident CSR vertex-to-face
adjacency) is the same parallel-build problem as the point clouds, and is better done once, with a
real consumer driving the design. When it happens: `FaceT::computeCentroid`/`computeNormal`/
`computeArea` rewritten as streaming half-edge walks, then the device-resident adjacency, then the
angle-weighted vertex pseudonormal. All three parts or none: the pseudonormal is what makes the sign
correct, so a device `reconcile()` covering only faces would leave signs silently wrong near
vertices and edges.

**More device backends (audit item 27).** SYCL first; OpenMP offload only if a user needs it;
OpenACC declined. Each is a real backend or none: a memory resource, device-pass detection, the GPU
test harness running on it, and a CI lane.

### Examples and integrations

The AMReX `MeshSDF`, `PaintEB`, `PackedSpheres` and `RandomCity` implicit functions are
`amrex::GPUable` and hold a host and a device descriptor of the same geometry, with the pool frozen
and mirrored in a GPU build. They were compiled and run on CPUs against AMReX's development branch
and compiled for HIP, but have not run on a device. The Chombo `MeshSDF`, `PackedSpheres` and
`RandomCity` hold their geometry by value and the pool by `shared_ptr`, and were compiled and run
against Chombo 3.2 (audit item 22). Still open: a GPU section in `Examples/MeshSDF` (mirror to
managed memory, evaluate the same points in a kernel, compare against the host).

## Decisions and their evidence

Kept so the decisions do not have to be re-derived. Most came out of the BVH port (#145, #146);
the measurements are from that time and the code has moved on since, as each item notes.

### Value types, not virtual interfaces

A class with virtual functions carries a pointer to a host-side virtual-function table, so it is
never trivially copyable however its members are stored, and CUDA forbids passing one to a kernel.
The original plan was to move each formula into a trait as a `static EBGEOMETRY_HOST_DEVICE eval()`
and keep the virtual `value()` as a thin delegate, for no API break. It does not work, for exactly
that reason: the delegate keeps the vtable. Porting `FlatMeshSDF` showed it, and every ported class
since is a plain value type with no base class: its payload held by value, no `shared_ptr`
constructors, an `EBGEOMETRY_HOST_DEVICE` `signedDistance()`, and `rebasedView()`/`deepCopy()`
returning the class itself. Their use as `ImplicitFunction`s (in CSG, the transforms and the
integrations) was dropped until the tape, with no interim host-side adapter (D5). The first attempt
had deleted `SignedDistanceFunction<T>` outright as part of the port, a user-visible break with no
GPU motivation; it was retired later on its own terms, once no built-in class implemented it (D2,
#164).

### One traversal loop

`pruneTraverse` used to hold seven copies of the same branch-and-bound loop: six `if constexpr` SIMD
blocks plus a scalar fallback that had no loop of its own, building four `std::function`s and
delegating to a heap-stack `traverse()`. Compared line by line, the six SIMD blocks differed only in
how the K per-child squared distances were computed, so that one expression became
`computeChildDistances2()` and there is one loop. The scalar case is an ordinary path through it
(fixed stack, hand-rolled insertion sort over the ≤ K children, no `std::function`, no heap). Every
path computes `max(0, max(lo - p, p - hi))` per axis and combines as `dx*dx + (dy*dy + dz*dz)`, so
all of them agree bit for bit, and a test sweeping K over values with and without a vector path
requires *exact* equality. That sweep found a latent bug: `ChildAABBSoA` asked for
`alignas(sizeof(T) * K)` unconditionally, which is not a legal alignment unless the product is a
power of two, so `PackedBVH<double, P, 3>` and its odd-K siblings had never compiled. Only the
`(T, K)` pairs with a SIMD path need the row aligned to its width; the rest take `alignof(T)`. The
pool keeps that alignment for free: `Pool::reserve` aligns each offset, and the block base is 256 B.

### No storage policy: `PackedBVH<T, uint32_t, K>` is the index

#145 replaced `PackedBVH`'s `shared_ptr` primitive array, which cannot be byte-copied to a device,
with a `StoragePolicy` parameter offering `ValueStorage` and `IndexStorage` (a `uint32_t` resolved
against a caller-owned array). #146 removed both again, after every justification for
`IndexStorage` failed. Measured on a 135,200-face watertight torus, `T = double`, `K = 4`, host,
single-threaded:

* **Packing reorders, so the pool cannot deduplicate.** Packing puts the primitives into leaf order —
  the property the packed layout exists to create — so a packed BVH can never alias the array it was
  built from, and two partitionings never agree on an order. Pool residency buys position
  independence and lifetime, not a shared ordering.
* **Sharing and instancing are already solved a layer up.** A `PackedBVH` copy is shallow: on an
  87,381-node BVH a copy is 64 bytes with a pool delta of zero, aliasing the source. A
  `PackedBVH` *is* an index — three `PODVector` offsets plus a location.
* **The tape needs nothing more.** Under the tape a union's primitive is a clause id, and
  `PackedBVH<T, uint32_t, K>` stores the same four bytes with no indirection through every
  construction path (the SFC, partitioner/SAH and cluster constructors and `TreeBVH::pack()`).
  `IndexStorage` would only have moved the indexing into the policy; `pruneTraverse` never called
  its resolver.
* **It reads more, not less.** A leaf evaluator still needs the face's normal, centroid and
  projection axes, so it touches all 88 bytes of a `FaceT` either way; the index adds a 4-byte
  read and makes the 88-byte one scattered.

`MeshSDF` later took the same route without any policy (#156): its BVH is a `PackedBVH` of
`uint32_t` face ids, each leaf test reading the face from the mesh, so the mesh is the only copy of
the geometry (88 bytes per face down to 4, at about 15% slower queries on the armadillo mesh). What
would bring a policy back: a consumer needing genuine *sharing* — several BVHs with different
partitionings over one primitive set, where editing a primitive once must reach all of them — and
even then, `PackedBVH<T, uint32_t, K>` gives that without policy machinery.

### Copies are shallow, and the packed BVH owns its primitives

A pool-resident object is descriptors plus a location, so a copy aliases the original's pool memory.
That shallowness is what makes the type trivially copyable and therefore mirrorable, and it inverts
what a copy constructor used to mean: `deepCopy(Pool&)` is the replacement. Storing primitives by
value also means mutating the source geometry no longer reaches a packed BVH, so `PackedBVH` has a
mutable `getPrimitives()` for moving a packed geometry in place before `refit()`.

### Leaf size, not storage, moves the footprint

Measured on the same torus, through the SFC constructor:

| target leaf | nodes | pool | faces evaluated per query |
|---|---|---|---|
| 1 | 349,525 | 99.3 MB | 613 |
| 2 | 349,525 | 99.3 MB | 1,898 |
| 4 | 87,381 | 33.3 MB | 1,053 |
| 16 | 21,845 | 16.8 MB | 4,510 |
| 64 | 5,461 | 12.7 MB | 4,745 |

* Going from leaf 1 to leaf 4 cut the pool by two thirds while getting slightly faster — roughly six
  times what `IndexStorage` could have saved, from an argument that already existed. The per-method
  leaf settings (`BVH::ConstructionOptions`, BVH-2, #161) came out of this.
* A target that was not a power of K was a silent trap: the SFC build picks a level count, so 2
  kept leaf 1's tree and packed twice the faces into each leaf. The SFC constructors now treat the
  target as a maximum and choose the fewest leaves that respect it (#161).
* About 75% of the `ChildAABBSoA` cache was zeros: one row per node, read only for interior nodes.
  Only interior nodes get a row now (#154).

These are one mesh, one K and one query pattern, and a torus flatters the SFC build; the structural
findings generalise, the specific numbers do not.

### Device stack depth

The traversal stack is fixed-size, so its depth is a budget: 256 levels on the host and 32 on a
device, at every K, in 8-byte entries (#153). A build deeper than the budget is rejected when the
BVH is built or rebased, rather than overflowing during a query.

## Porting a class

1. Make every member a plain value. Cross-references become `uint32_t` indices into the owning
   container's arrays (sentinel `UINT32_MAX`), resolved by passing that owner in explicitly — never
   a cached pointer, which is only valid in the address space that set it.
2. Move array storage onto `PODVector` reserved from a caller-supplied `Pool`. Build-phase mutators
   take the `Pool&` explicitly, since `reserve()` can move the base.
3. Annotate: `EBGEOMETRY_HOST_DEVICE` for anything that resolves purely through values, indices and
   other device-callable calls; `EBGEOMETRY_HOST` for anything touching a `Pool` (including
   `rebasedView`, which reads one), `std::vector`, `std::string`, `std::cerr` or `std::map`. Use
   `Math::min`/`max`/`clamp`/`Limits` and `Array<T, N>` instead of their `std` counterparts, and
   call no other `std::` function in device code except the math functions
   (`Scripts/CheckDeviceMath.py` checks both; see "Writing device code" in the contribution
   guidelines).
4. `static_assert(std::is_trivially_copyable_v<…>)` on the class, and on any user type parameter
   it stores in pool memory.
5. If a container-returning method (`std::vector`) is genuinely useful on device, add a *streaming*
   sibling rather than trying to annotate it — see
   `FaceT::getSmallestCoordinate`/`getHighestCoordinate`.
6. Add the class to `Tests/InstantiateAll.cpp`, add a `[gpu]`-tagged test case that evaluates it on
   many queries with the `Tests/TestGPU.hpp` harness and compares element by element against the
   host (see "Adding tests" in the contribution guidelines), and add the test binary to
   `EBGEOMETRY_GPU_TESTS` in `Tests/CMakeLists.txt`.
7. Update the Sphinx page that documents the class, per the rules in `CLAUDE.md`.

## Verifying locally

The `cuda` and `hip` presets compile the device-bearing tests. Compilation needs only a toolkit; the
`[gpu]` cases additionally need a visible device and `SKIP()` cleanly without one. Every host build
also runs the `[gpu]` cases in emulation, as above.

```bash
cmake --preset cuda -DCMAKE_CUDA_ARCHITECTURES=<arch>   # match the local GPU; preset default is 70
cmake --build build/cuda --target EBGeometry_GPUDeviceTests --parallel $(nproc)
cd build/cuda && ctest -L gpu-device --output-on-failure
```

Without a CUDA or ROCm toolkit, the same clang HIP invocation CI's `GPU-HIP` job uses (see
`.github/workflows/CI.yml`) installs from the distribution's packages and compiles every device
test for a real AMD target with no GPU present. It needs `EBGEOMETRY_ENABLE_ASSERTIONS=OFF` (the
distribution's HIP headers provide no device-side `assert`), and it runs nothing.

CI compiles both backends on every push but has no physical GPU, so the device-side assertions only
ever execute on a developer machine with one: a green GPU lane means "it compiles", nothing more.
`GPU-HIP` is required by `CI-passed`; `GPU-CUDA` is advisory (`continue-on-error`), so read that
lane itself.
