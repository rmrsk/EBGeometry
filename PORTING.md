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

## What is not

| Component | Blocker |
|---|---|
| `TreeBVH` | Host-only **by design** — it is the builder, and static geometry builds on the host. Not a gap. |
| `MeshSDF` / `FlatMeshSDF` / `TriMeshSDF` | Their `PackedBVH` is ported and pool-backed, but the wrappers are not device-callable and, more to the point, not trivially copyable: each holds its BVH and/or mesh as a `shared_ptr` member. Next step |
| `Triangle<T, Meta>` (AoS), `Octree` | Not started |
| `PointCloudBVH` | **Half-ported, and currently unsound to mirror.** Its inherited `PackedBVH` arrays are pool-backed, but its own cloud arrays (`m_positions`, `m_metadata`, `m_order`, `m_leafOff`, `m_leafCnt`) are still `std::vector`, so the class is not trivially copyable — while the `rebasedView()`/`deepCopy()` it inherits *are* callable on it and silently slice to the base. See the roadmap's step 0b |
| `PointCloudHashGrid`, `SFC` | Not started; the point-cloud BVH additionally has to *build* on device |
| `ImplicitFunction`, `CSG`, `Transform`, analytic SDFs | Still the original virtual-`value()` design; this is where the tape returns |
| `BVHUnionIF` / `BVHSmoothUnionIF` | **Compiled out** behind `EBGEOMETRY_ENABLE_BVH_CSG_UNION`. They stored polymorphic primitives as `shared_ptr`, which no trivially-copyable storage policy can hold; they return with the index-based CSG redesign in step 4 |
| Parsers (`OBJ`/`PLY`/`STL`/`VTK`/`Soup`), `Random`, `SimpleTimer` | Host-only by design — no port intended |

## Roadmap

> While it exists, `PLAN.md` in this directory carries the detail: what the BVH port (step 2)
> actually did and where the plan was wrong about the code, then the agreed design for the mesh SDFs,
> the implicit-function layer and the tape. It settles several questions this page only lists.
> `PLAN.md` is deleted once that work lands, at which point whatever is still true moves here.

**The numbered steps below are kept in their original order for continuity; the order they are being
*done* in is different.** Actual sequence, agreed after the BVH port:

> step 2 (done) → **0a** → **0b** → step 4 restricted to the mesh SDFs → step 4 proper → step 5 →
> steps 1, 3, 6 and the remaining loose ends.

0a. **Prerequisite: a CUDA/HIP toolkit on the development machine.** There is none at present (a GPU
   is present; `nvcc` is not installed), so neither the `cuda` nor the `hip` preset configures and no
   `[gpu]` case can be compiled locally. This is not hypothetical: two `PackedBVH` accessors were
   found missing their `EBGEOMETRY_HOST_DEVICE` annotations *after* a `[gpu]` kernel calling one of
   them had been written, by reading the code rather than building it. Everything from step 4 onward
   adds far more device code than the BVH port did.

0b. **`PointCloudBVH` should consume a `PackedBVH`, not derive from one.** `PackedBVH` documents
   itself as "not intended to be subclassed"; `PointCloudBVH` subclasses it anyway. Since the BVH
   port added `rebasedView()`/`deepCopy()` returning `PackedBVH` **by value**, both are silently
   sliced on the derived type — mirroring a point cloud compiles cleanly and produces a BVH whose
   leaves reference a cloud that was never copied. Hold the BVH by value as a member instead (it is
   trivially copyable now), move the cloud arrays onto `PODVector`, give the class its own
   `rebasedView()`, and mark `PackedBVH` `final`. Worth doing before the mesh SDFs because it is the
   same BVH-plus-payload rebase that `MeshSDF` needs, on a smaller subject.

1. **DCEL reconcile chain.** *(Deferred deliberately.)* Not a prerequisite for anything else.
   `MeshT::reconcile()`'s only production call site is `Soup::readIntoDCEL` (`SoupImplem.hpp`), and
   `Soup` is host-only by design — so a device `reconcile()` would today have no device caller. Its
   hard part (device-resident CSR vertex→face adjacency) is the same parallel-build problem as step 3
   and is better done once, with a real consumer driving the design.

   The work itself, when it happens: `FaceT::computeCentroid`/`computeNormal`/`computeArea` rewritten
   as streaming half-edge walks (they currently open with `gatherVertexIndices()`, which materializes
   a `std::vector`), then device-resident CSR vertex→face adjacency, then
   `VertexT::computeVertexNormalAngleWeighted`. All three parts or none: the angle-weighted
   pseudonormal is what makes the sign correct, so a device `reconcile()` covering only faces would
   leave signs silently wrong near vertices and edges.
2. **BVH.** *(Done, on branch `bvh_port_PR1` / PR #145.)* `pruneTraverse` factored into one loop with
   a real scalar implementation (fixed stack, hand-rolled sort over the ≤K children);
   `SharedPtrStorage` dropped in favour of two POD policies (`ValueStorage`, and a new
   `IndexStorage`); `PackedBVH`'s three arrays moved onto `Pool`/`PODVector`; the class is
   `static_assert`-ed trivially copyable and has `rebasedView()`/`deepCopy()`; stack depth differs by
   entry point (256 host, 64 device). `TreeBVH` stays host-only — it is the builder, and static
   geometry builds on the host.

   Two things did **not** land with it. The mesh SDF wrappers are step 4 below rather than part of
   this step. And `BVHUnionIF`/`BVHSmoothUnionIF` had to be compiled out, because they store
   polymorphic primitives as `shared_ptr` and no trivially-copyable policy can hold those — see the
   "What is not" table and step 4.
3. **Point clouds.** Device-side `PointCloudBVH` build (Morton codes + radix sort) and a
   `PointCloudHashGrid` counterpart. Independent of 1–2.
4. **Standalone, tag-nameable types: the mesh SDFs first, then the analytic SDFs / transforms /
   combiners.** Each type's formula moves into a trait as a `static EBGEOMETRY_HOST_DEVICE eval()`,
   with the existing virtual `value()` becoming a thin delegate — no API break, and a kernel can call
   `eval()` directly, giving real device coverage before any tape exists.

   **Do this in two passes.** First the mesh distance functions (`FlatMeshSDF`, then `TriMeshSDF`,
   then `MeshSDF` — ascending by number of moving parts), because their primitives and BVH are
   already ported and `FlatMeshSDF` yields a device-side brute-force oracle to validate the rest
   against. Then the analytic layer proper, which is where `BVHUnionIF`/`BVHSmoothUnionIF` come back
   and where re-enabling `EBGEOMETRY_ENABLE_BVH_CSG_UNION` is the acceptance test.

   **De-virtualising is only half of it.** Every one of these classes also holds its payload as a
   `shared_ptr` member (`FlatMeshSDF`: the mesh; `TriMeshSDF`: the BVH; `MeshSDF`: both). Annotating
   methods changes nothing while those remain — the class stays non-trivially-copyable and cannot be
   mirrored. They must become by-value members, which is newly possible: `PackedBVH` and
   `DCEL::MeshT` are both trivially copyable as of step 2.

   **Define the tag/opcode registry once, in the first pass**, even though only a few types populate
   it at first, so the mesh SDFs and the analytic layer do not end up with two schemes to merge.

   **`SignedDistanceFunction<T>` stays**, as does `ImplicitFunction<T>`: they are the compatibility
   surface the thin delegates hang off. The first attempt deleted `SignedDistanceFunction<T>` outright
   and collapsed everything onto `ImplicitFunction<T, Op>` + `bool m_sdf`, which was a user-visible
   API break with no GPU motivation of its own. Revisit that as a deliberate change on its own terms,
   not as a side effect of this step.
5. **The tape.** The linear-SSA clause list and interpreter that replaces virtual dispatch, built on
   `Pool`/`PODVector` from the start rather than on `std::vector` with an upload path bolted on. It
   depends on step 4 for its opcodes and on step 2 for the BVH-union opcodes, which reference the
   packed BVH arrays directly.
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
   `std::string`, `std::cerr` or `std::map`. Watch for standard-library helpers that look innocent —
   `std::clamp` pulls in a host-only assert handler under libstdc++ hardened mode and must be
   hand-rolled.
4. `static_assert(std::is_trivially_copyable_v<…>)` on the class, and on any `Meta` template
   parameter it stores.
5. If a container-returning method (`std::vector`) is genuinely useful on device, add a *streaming*
   sibling rather than trying to annotate it — see
   `FaceT::getSmallestCoordinate`/`getHighestCoordinate`.
6. Add the class to `Tests/InstantiateAll.cpp`, add a `[gpu]`-tagged test case that launches a
   kernel and compares against the host, and add the test binary to `EBGEOMETRY_GPU_TESTS` in
   `Tests/CMakeLists.txt`.
7. Update the Sphinx page that documents the class, per the rules in `CLAUDE.md`.

## Verifying locally

The `cuda` and `hip` presets compile the device-bearing tests. Compilation needs only a toolkit; the
`[gpu]` cases additionally need a visible device and `SKIP()` cleanly without one.

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
optional — a green CI GPU lane means "it compiles", nothing more.
