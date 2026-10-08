# Porting EBGeometry to GPUs

Working overview of the GPU port: where it stands, the one design rule everything follows, what a
first attempt got wrong, and what remains. This file is the authoritative status document —
[issue #115](https://github.com/rmrsk/EBGeometry/issues/115) still holds the original design
discussion and is worth reading for the *rationale* behind individual decisions, but its tier
checklist describes the abandoned first attempt and is **stale**. Where the two disagree, this file
wins.

Status as of 2026-09-08. `dev` is at `e6914d4`; the BVH port (roadmap step 2) is on branch
`bvh_port_PR1` / [PR #145](https://github.com/rmrsk/EBGeometry/pull/145) and is reflected below.

## The one rule

> **Store an offset, not a pointer. The base is supplied by the caller.**

Every other convention in the port follows from this. A pointer that is valid in host address space
is meaningless once the same bytes are copied to a device, so no structure that has to cross may
contain one. Structures instead store byte offsets relative to a base address passed in separately,
which makes the identical bit pattern resolve correctly against a host base *and* a device base —
with no pointer-patching pass after the copy.

The practical consequence is that a portable structure has **at most one** address-space-specific
field, and it is set at the moment of crossing rather than being a property of the object. For
`DCEL::MeshT` that field is `m_base`, and `MeshT::rebasedView(pool)` is the sanctioned way to
produce a copy that resolves in a different address space:

```cpp
EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(hostPool, EBGeometry::deviceMemoryResource());
const auto       deviceMesh = mesh->rebasedView(devicePool);        // rebase on the host …
myKernel<<<blocks, threads>>>(deviceMesh, …);                       // … then copy by value
```

Rebase, then copy — never copy a host-resident value and try to repair it afterwards.

A host-resident object does *not* hold a base at all: it holds a pointer to its `Pool`'s control
block and re-reads the base through it on every access. That is what makes it immune to a
`reserve()` that grows and moves the block, and what makes `m_control == nullptr` mean "device
view" and nothing else — the assertion both `base()` branches rely on.

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

This is also why the current working agreement forbids *host/device data duality*: one flat,
trivially-copyable POD type used on both sides, never a `std::vector` host representation paired
with a POD device mirror. `std::vector` is acceptable only in host-only build steps that never
cross, and those are marked `EBGEOMETRY_HOST`.

### What survived

Not everything was discarded. The backend portability layer — `EBGeometry_GPU.hpp` (the
`EBGEOMETRY_HOST` / `EBGEOMETRY_HOST_DEVICE` / `EBGEOMETRY_GLOBAL` decorations, device-safe
`EBGEOMETRY_EXPECT`) and `EBGeometry_GPURuntime.hpp` (the backend-neutral `EBGeometry::GPU::*`
wrappers over the CUDA/HIP runtime) — was carried forward into #130, extended only with the managed,
pinned and mapped allocation wrappers the new `MemoryResource` types need. It is what the current
tests and mirroring run on.

The *evaluation* design also stands and is expected to return once the data layer reaches it: a
linear-SSA tape in the MPR/libfive style, one trait per primitive as the single source of truth for
its formula, a generated opcode registry, smooth-blend operators as template parameters rather than
`std::function`, and a fixed `uint32` winning-primitive index as the only thing transported per SSA
slot. Those decisions and their reasoning are recorded in the comments on issue #115; they do not
need re-deriving.

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
distinction, no freeze-before-query rule, and no base parameter threaded through the API;
`rebasedView(pool)` is the single crossing point. `DCEL::MeshT` is the reference implementation of
the pattern.

Two rules survive from this and must be documented on anything that adopts it: a resolved address
(a reference, a `PODSpan`) must not outlive the next `reserve()` on its pool, and the pool must
outlive every object reserved from it.

## What is ported

Device-callable, trivially copyable, and covered by a `[gpu]`-tagged test that launches a real
kernel and compares against the host:

| Component | Headers | PR |
|---|---|---|
| Memory foundation | `MemoryResource`, `Pool`, `PODVector` | #130 |
| Vectors | `EBGeometry_Vec.hpp` | #131 |
| Bounding volumes | `EBGeometry_BoundingVolumes.hpp` (`AABBT`, `SphereT`) | #132, #133 |
| SoA/AoSoA leaves | `PointSoA`, `PointAoSoA`, `TriangleSoA`, `TriangleAoSoA` | #134 |
| DCEL | `VertexT`, `EdgeT`, `FaceT`, `EdgeIteratorT`, `MeshT` | #137–#140 |
| BVH traversal + storage | `PackedBVH` (`Node`, `ChildAABBSoA`, `pruneTraverse`) | this branch |
| Point-cloud queries | `PointCloudBVH` (holds a `PackedBVH`) and `PointCloudHashGrid` (pool-backed CSR grid); both builds stay host-side. One `PointCloud::Hit` with a `uint32` cloud index, a shared `PointCloud::KBest`, device-callable brute-force references | roadmap step 0b; item 20 |
| Mesh SDFs | `FlatMeshSDF`, `MeshSDF`, `TriMeshSDF` (plain value types; no longer `SignedDistanceFunction`s; `getClosestFace` is device-callable and returns a face id) | roadmap step 4a |
| Analytic shapes | The twelve classes in `EBGeometry_AnalyticDistanceFunctions.hpp` (plain value types; no longer `SignedDistanceFunction`s; constructors stay host-only) | roadmap step 4b, first PR |
| BVH unions | `BVHUnionIF`, `BVHSmoothUnionIF` over one primitive type (plain value types; no longer `ImplicitFunction`s); `SmoothMinOp`/`SmoothMaxOp`/`ExpMinOp`; `PoolLocation` relocation of pool-resident primitives | roadmap step 4b, second PR |

## What is not

| Component | Blocker |
|---|---|
| `TreeBVH` | Host-only **by design** — it is the builder, and static geometry builds on the host. Not a gap. |
| `Triangle<T>` (AoS), `Octree` | Not started |
| `SFC`, point-cloud builds | Not started; the point-cloud BVH and hash grid additionally have to *build* on device |
| `ImplicitFunction`, `CSG`, `Transform` | Still the original virtual-`value()` design, now fed by user-written implicit functions only; this is where the tape returns. `approximateBoundingVolumeOctree` is a host-only free function taking any shape, mesh SDF, implicit function or callable |
| Unions of different primitive types | Need runtime dispatch, which is the tape. The `CSGUnion` example (a mesh plus a sphere) stays disabled until then |
| Parsers (`OBJ`/`PLY`/`STL`/`VTK`/`Soup`), `Random`, `SimpleTimer` | Host-only by design — no port intended |

## Roadmap

> While it exists, `PLAN.md` in this directory carries the detail: what the BVH port (step 2)
> actually did and where the plan was wrong about the code, then the agreed design for the mesh SDFs,
> the implicit-function layer and the tape. It settles several questions this page only lists.
> `PLAN.md` is deleted once that work lands, at which point whatever is still true moves here.

**The numbered steps below are kept in their original order for continuity; the order they are being
*done* in is different.** The governing rule: **every existing class is ported before the tape is
started.** Actual sequence:

> step 2 (done) → **0b** (done) → step 4 restricted to the mesh SDFs (done) → step 4 proper (analytic
> shapes and single-type BVH unions, done) → step 3 (point clouds) → the loose ends (`Triangle`,
> `Octree`, `SFC`) → step 1 (DCEL reconcile) → step 6 (GPU examples, AMReX integration) →
> **the pre-tape audit** → **step 5 (the tape), last**.
>
> **The pre-tape audit.** The state reached once every existing class is ported is a checkpoint.
> Before the tape is started, the whole codebase is audited -- every header, test, example,
> integration and document, not only what the port touched -- and whatever it finds is fixed
> before step 5 begins. Items already noted for it: the `IF` suffix on `BVHUnionIF` and
> `BVHSmoothUnionIF`, which are no longer `ImplicitFunction`s; the `Integrations/` examples still
> written against the virtual CSG interface; the `CSGUnion` example's disabled code; and
> `SignedDistanceFunction<T>`, which no built-in class implements any more.
>
> The audit was run early, after step 4, rather than after steps 3, 1 and 6: its findings reshape
> those steps, including several items to retire rather than port. The findings, the design review
> and the fix plan are in [AUDIT.md](AUDIT.md).
>
> **0a** (a CUDA/HIP toolkit on the development machine) is not a sequence step: it is still open
> and should be closed as early as possible, since every step after it adds device code.

One consequence of porting step 4 before the tape: `BVHUnionIF`/`BVHSmoothUnionIF` came back in
step 4 restricted to one primitive type. A union over primitives of *different* concrete types has
no way to pick each primitive's formula on a device without some form of runtime dispatch -- which
is what the tape is -- so until step 5 there is no such union at all; homogeneous ones (every
primitive the same concrete type) need no dispatch and run on a device. No interim dispatch
mechanism is to be built in the meantime, since it would be a second tape.

0a. **Prerequisite: a CUDA/HIP toolkit on the development machine.** There is none at present (a GPU
   is present; `nvcc` is not installed), so neither the `cuda` nor the `hip` preset configures and no
   `[gpu]` case can be compiled locally. This is not hypothetical: two `PackedBVH` accessors were
   found missing their `EBGEOMETRY_HOST_DEVICE` annotations *after* a `[gpu]` kernel calling one of
   them had been written, by reading the code rather than building it. Everything from step 4 onward
   adds far more device code than the BVH port did.

   *Stopgap, not a fix:* the same clang-17 HIP invocation CI's `GPU-HIP` job uses (see
   `.github/workflows/CI.yml`) installs from the distribution's packages and compiles every `[gpu]`
   kernel for a real AMD target with no GPU present. That is how step 0b's device code was
   compiled before it was pushed. It catches missing annotations and non-device-callable calls; it
   runs nothing, and it needs `EBGEOMETRY_ENABLE_ASSERTIONS=OFF` (the distribution's HIP headers
   provide no device-side `assert`). Actually executing a kernel still needs the toolkit this step
   asks for.

0b. **Done.** `PointCloudBVH` now holds its `PackedBVH` by value, keeps its cloud arrays as `PODVector`s
   in the same pool, is trivially copyable, and has its own `rebasedView()`/`deepCopy()` returning
   `PointCloudBVH`; `PackedBVH` is `final`. The original rationale follows.

   **`PointCloudBVH` should consume a `PackedBVH`, not derive from one.** `PackedBVH` documents
   itself as "not intended to be subclassed"; `PointCloudBVH` subclasses it anyway. Since the BVH
   port added `rebasedView()`/`deepCopy()` returning `PackedBVH` **by value**, both are silently
   sliced on the derived type — mirroring a point cloud compiles cleanly and produces a BVH whose
   leaves reference a cloud that was never copied. Hold the BVH by value as a member instead (it is
   trivially copyable now), move the cloud arrays onto `PODVector`, give the class its own
   `rebasedView()`, and mark `PackedBVH` `final`. Worth doing before the mesh SDFs because it is the
   same BVH-plus-payload rebase that `MeshSDF` needs, on a smaller subject.

1. **DCEL reconcile chain.** *(Deferred deliberately.)* Not a prerequisite for anything else.
   `MeshT::reconcile()`'s only production call site is `Soup::soupToDCEL` (`SoupImplem.hpp`), and
   `Soup` is host-only by design — so a device `reconcile()` would today have no device caller. Its
   hard part (device-resident CSR vertex→face adjacency) is the same parallel-build problem as step 3
   and is better done once, with a real consumer driving the design.

   The work itself, when it happens: `FaceT::computeCentroid`/`computeNormal`/`computeArea` rewritten
   as streaming half-edge walks (they currently open with `gatherVertexIndices()`, which materializes
   a `std::vector`), then device-resident CSR vertex→face adjacency, then
   `VertexT::computeVertexNormalAngleWeighted`. All three parts or none: the angle-weighted
   pseudonormal is what makes the sign correct, so a device `reconcile()` covering only faces would
   leave signs silently wrong near vertices and edges.
2. **BVH.** *(Done, PR #145, with a follow-up scrub.)* `pruneTraverse` factored into one loop with
   a real scalar implementation (fixed stack, hand-rolled sort over the ≤K children); `PackedBVH`'s
   three arrays moved onto `Pool`/`PODVector`; the class is `static_assert`-ed trivially copyable and
   has `rebasedView()`/`deepCopy()`; stack depth differs by entry point. (Since then: the stack holds
   256 levels on the host and 32 on a device at every K, in 8-byte entries, and only interior nodes
   get a SIMD child-box row.)
   `TreeBVH` stays host-only — it is the builder, and static geometry builds on the host.

   The `shared_ptr`-based primitive array is gone, since a `shared_ptr` cannot be byte-copied into a
   device address space. PR #145 replaced it with a `StoragePolicy` template parameter offering
   `ValueStorage` and `IndexStorage`; the follow-up scrub removed that parameter again, after
   `IndexStorage` turned out to have no use a `PackedBVH<T, uint32_t, K>` does not serve more
   cheaply (`PLAN.md`'s "PR D" carries the measurements). `PackedBVH` now stores its primitives by
   value, full stop, and an indexed BVH is just one whose primitive type is `uint32_t`.

   Two things did **not** land with it. The mesh SDF wrappers are step 4 below rather than part of
   this step. And `BVHUnionIF`/`BVHSmoothUnionIF` had to be compiled out, because they store
   polymorphic primitives as `shared_ptr`, which a by-value primitive array cannot hold — see the
   "What is not" table and step 4.

   **The device traversal is correctness-first, and is not a tuned GPU kernel.** It is the textbook
   formulation: one query point per thread, each with a private stack, every lane descending
   its own path. That is divergence-bound by construction — the warp executes the union of 32
   different descents and retires with the slowest lane — and the private stack is local memory
   (752 B/thread at K = 4), touched on every push and pop. `ChildAABBSoA` is also
   laid out for the wrong axis here: it exists so one *thread* can load K children into one SIMD
   register, which is a CPU idea and buys nothing when a lane reads all K serially.

   None of that is measured — see step 0a; the kernel has never been compiled. It is recorded so the
   layout is not mistaken for a finished design, and so nobody benchmarks it and draws a conclusion
   about the library's GPU ceiling. Tuning is deliberately deferred; the options, cheapest first, are
   Morton-sorting queries before launch (no kernel change at all), warp-cooperative traversal (one
   warp per query, K children one-per-lane — which is what would finally make the SoA layout pay off
   on device), stackless traversal, and persistent threads with a work queue. **Measure with
   spatially coherent queries before choosing**: the real consumers generate query points cell-by-cell
   over a grid, which is far more coherent than the random-point worst case this analysis assumed.
3. **Point clouds.** Device-side `PointCloudBVH` build (Morton codes + radix sort) and a
   `PointCloudHashGrid` counterpart (both already query on device). Independent of 1–2.
4. **Standalone, tag-nameable types: the mesh SDFs first, then the analytic SDFs / transforms /
   combiners.** Each type's formula moves into a trait as a `static EBGEOMETRY_HOST_DEVICE eval()`,
   with the existing virtual `value()` becoming a thin delegate — no API break, and a kernel can call
   `eval()` directly, giving real device coverage before any tape exists.

   **Do this in two passes.** First the mesh distance functions (`FlatMeshSDF`, then `TriMeshSDF`,
   then `MeshSDF` — ascending by number of moving parts), because their primitives and BVH are
   already ported and `FlatMeshSDF` yields a device-side brute-force oracle to validate the rest
   against. Then the analytic layer proper, which is where `BVHUnionIF`/`BVHSmoothUnionIF` come back
   and where re-enabling `EBGEOMETRY_ENABLE_BVH_CSG_UNION` is the acceptance test.

   **The BVH side of this is already in place.** Under the tape a union's primitive is a clause id,
   so the union's BVH is a `PackedBVH<T, uint32_t, K>`: four bytes per primitive, no indirection, and
   it already builds through all four construction paths — the SFC, partitioner/SAH and `ClusterSpec`
   constructors and `TreeBVH::pack()` — and is trivially copyable, verified against the tree.
   `PLAN.md`'s "PR D" section carries the evidence, including why the `IndexStorage` policy that once
   wrapped this pattern bought nothing over it.

   **The mesh SDFs stop deriving from `SignedDistanceFunction`.** A class with virtual functions
   carries a pointer to a host-side virtual-function table, so it is never trivially copyable however
   its members are stored, and CUDA forbids passing one to a kernel. Each mesh SDF therefore becomes a
   plain value type: its payload held by value (`PackedBVH`, `DCEL::MeshT`, both trivially copyable as
   of step 2), constructed without `shared_ptr`s, with an `EBGEOMETRY_HOST_DEVICE` `signedDistance()`,
   and with `rebasedView(pool)`/`deepCopy(pool)` returning the class itself. Parser entry points return
   them by value, as do `Parser::readIntoDCEL`, `readIntoTriangles` and `DCEL::MeshT::deepCopy`. All
   three mesh SDFs are done (step 4a). This deliberately drops their use as `ImplicitFunction`s (in
   CSG, transforms and the AMReX/Chombo integrations) until the tape brings composition back; no
   host-side adapter is to be built in the meantime.

   **The analytic shapes follow the mesh SDFs exactly (step 4b, first PR, done).** All twelve are
   plain, trivially copyable value types with an `EBGEOMETRY_HOST_DEVICE` `signedDistance()`; the
   "formula in a trait, virtual `value()` as a thin delegate" pattern above was dropped for them for
   the same vtable reason. `RoundedBoxSDF` holds its sphere by value, and `PerlinSDF`'s helpers are
   no longer virtual. The shapes are therefore no longer `ImplicitFunction`s: the transforms and CSG
   combinators keep their virtual interface and accept user-written implicit functions only, with no
   adapter, until the tape. The transform/CSG tests present shapes as `ImplicitFunction`s through a
   test-only helper (`Tests/TestShapeIF.hpp`), and `approximateBoundingVolumeOctree` became a free
   function taking anything with a `signedDistance()`, a `value()` or a call operator.

   **The BVH unions, restricted to one primitive type (step 4b, second PR, done).** A
   `BVHUnionIF<T, P, K>` is a `PackedBVH<T, P, K>` over value-type primitives of one type (`SphereSDF`,
   `BoxSDF`, `TriMeshSDF`, another union, ...), trivially copyable and device-callable, with no
   `ImplicitFunction` base. The smooth-min/max blends became copyable function objects
   (`SmoothMinOp` and friends; `SmoothMin<T>` is now an instance of one), and
   `EBGEOMETRY_ENABLE_BVH_CSG_UNION` is gone. The one new mechanism is `PoolLocation`: a
   pool-resident primitive (a mesh SDF, or a nested union) is stored byte for byte in the union's
   pool, host control block included, so the union applies its own location to a copy of each such
   primitive as it evaluates it (`relocatedTo()`, on `PackedBVH`, `MeshT` and the mesh SDFs). A test
   evaluates mirror and deep copies of a mesh union after destroying the source pool, which fails
   under ASan without the relocation. This revised the acceptance test above: `RandomCity`,
   `PackedSpheres` and `NestedBVH` (with the mesh translated before building rather than through
   `Translate`) are back, but `CSGUnion` -- a union of a mesh and a sphere, i.e. of different
   types -- stays disabled until the tape.

   **The tag/opcode registry is deferred to the tape.** It was to be defined in the first pass so the
   mesh SDFs and the analytic layer shared one scheme, but only the tape consumes it, and the tape is
   now the last step; defining it there avoids designing it before its one consumer exists.

   **`SignedDistanceFunction<T>` stays**, as does `ImplicitFunction<T>`: they are the interface the
   transforms and CSG combinators (and user-written implicit functions) still use. The first attempt deleted `SignedDistanceFunction<T>` outright
   and collapsed everything onto `ImplicitFunction<T, Op>` + `bool m_sdf`, which was a user-visible
   API break with no GPU motivation of its own. Revisit that as a deliberate change on its own terms,
   not as a side effect of this step.
5. **The tape.** The linear-SSA clause list and interpreter that replaces virtual dispatch, built on
   `Pool`/`PODVector` from the start rather than on `std::vector` with an upload path bolted on. It
   depends on step 4 for its opcodes and on step 2 for the BVH-union opcodes, which reference the
   packed BVH arrays directly.

   **After this, not before: a batched traversal path.** The device traversal step 2 shipped is
   correctness-first (see its note above), and the eventual fix is a second traversal loop. The
   agreed shape, recorded here so the reasoning survives the gap:

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
   * **Do not build it before there are measurements.** Try Morton-sorting queries before launch
     first; it attacks divergence and locality together and needs no kernel change at all. And measure
     a *representative* workload: the real consumers (AMReX/Chombo EB generation) evaluate points
     cell-by-cell over a grid, which is already close to Morton order, so a random-point benchmark
     would overstate divergence badly and could justify a rebuild on false evidence.
   * **Why after the tape and not before it.** Step 5 changes what a leaf evaluator is, so a batched
     traversal built earlier gets partly rebuilt anyway.

   Nothing needs doing now to keep this open: `pruneTraverse`'s contract already says nothing about
   threads. The one thing worth adding alongside the first `[gpu]` benchmark is a coherent-query
   case, so the choice of formulation has evidence behind it.
6. **Examples and integrations.** A GPU section in `Examples/MeshSDF` (mirror to managed memory,
   evaluate the same points in a kernel), and an AMReX integration exercising device evaluation
   end to end.

## Porting a class

1. Make every member a plain value. Cross-references become `uint32_t` indices into the owning
   container's arrays (sentinel `UINT32_MAX`), resolved by passing that owner in explicitly — never
   a cached pointer, which is only valid in the address space that set it.
2. Move array storage onto `PODVector` reserved from a caller-supplied `Pool`. Build-phase mutators
   take the `Pool&` explicitly, since `reserve()` can move the base.
3. Annotate: `EBGEOMETRY_HOST_DEVICE` for anything that resolves purely through values, indices and
   other device-callable calls; `EBGEOMETRY_HOST` for anything touching a `Pool` (including
   `rebasedView`, which reads one), `std::vector`,
   `std::string`, `std::cerr` or `std::map`. Use `Math::min`/`max`/`clamp`/`Limits` and
   `Array<T, N>` instead of their `std` counterparts, and call no other `std::` function in device
   code except the math functions (`Scripts/CheckDeviceMath.py` checks both; see "Writing device
   code" in the contribution guidelines).
4. `static_assert(std::is_trivially_copyable_v<…>)` on the class, and on any user type parameter
   it stores in pool memory.
5. If a container-returning method (`std::vector`) is genuinely useful on device, add a *streaming*
   sibling rather than trying to annotate it — see
   `FaceT::getSmallestCoordinate`/`getHighestCoordinate`.
6. Add the class to `Tests/InstantiateAll.cpp`, add a `[gpu]`-tagged test case that evaluates it on
   many queries with the `Tests/TestGPU.hpp` harness and compares element by element against the
   host (see "Adding tests" in the contribution guidelines), and add the test binary to `EBGEOMETRY_GPU_TESTS` in
   `Tests/CMakeLists.txt`.
7. Update the Sphinx page that documents the class, per the rules in `CLAUDE.md`.

## Verifying locally

The `cuda` and `hip` presets compile the device-bearing tests. Compilation needs only a toolkit; the
`[gpu]` cases additionally need a visible device and `SKIP()` cleanly without one. Every host build
also runs the `[gpu]` cases, in emulation (host memory that reports itself device-accessible, and a
host loop in place of the kernel), so a device test that fails for a reason other than the device
compile or the launch already fails in the ordinary test suite.

```bash
cmake --preset cuda -DCMAKE_CUDA_ARCHITECTURES=<arch>   # match the local GPU; preset default is 70
cmake --build build/cuda --target EBGeometry_GPUDeviceTests --parallel $(nproc)
cd build/cuda && ctest -L gpu-device --output-on-failure
```

> **This does not currently work on the development machine.** A GPU is present but no CUDA toolkit
> is installed, so `cmake --preset cuda` cannot configure and no `[gpu]` case has been compiled since
> the BVH port. Roadmap step 0a exists to fix that, and it should be fixed before more device code is
> written: two `PackedBVH` accessors were found missing their `EBGEOMETRY_HOST_DEVICE` annotations
> only by reading the code, *after* a kernel calling one of them had already been written and
> committed.

CI compiles both backends on every push but has no physical GPU, so the device-side assertions only
ever execute on a developer machine. Running the above before pushing GPU work is therefore not
optional — a green CI GPU lane means "it compiles", nothing more. Note also that both GPU jobs are
`continue-on-error` and are not among `CI-passed`'s requirements, so a PR that breaks device
compilation still shows a green overall check; read the GPU lanes themselves.
