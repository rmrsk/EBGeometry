# Porting EBGeometry to GPUs

Working overview of the GPU port: where it stands, the one design rule everything follows, what a
first attempt got wrong, and what remains. This file is the authoritative status document —
[issue #115](https://github.com/rmrsk/EBGeometry/issues/115) still holds the original design
discussion and is worth reading for the *rationale* behind individual decisions, but its tier
checklist describes the abandoned first attempt and is **stale**. Where the two disagree, this file
wins.

Status as of 2026-08-12, `dev` at `fae8eeb`.

## The one rule

> **Store an offset, not a pointer. The base is supplied by the caller.**

Every other convention in the port follows from this. A pointer that is valid in host address space
is meaningless once the same bytes are copied to a device, so no structure that has to cross may
contain one. Structures instead store byte offsets relative to a base address passed in separately,
which makes the identical bit pattern resolve correctly against a host base *and* a device base —
with no pointer-patching pass after the copy.

The practical consequence is that a portable structure has **at most one** address-space-specific
field, and it is set at the moment of copying rather than being a property of the object. For
`DCEL::MeshT` that field is `m_base`, and `MeshT::boundView(base)` is the sanctioned way to produce
a copy bound to a different address space:

```cpp
EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(hostPool, EBGeometry::deviceMemoryResource());
const auto       deviceMesh = mesh->boundView(devicePool.base());   // rebase on the host …
myKernel<<<blocks, threads>>>(deviceMesh, …);                       // … then copy by value
```

Rebase, then copy — never copy a host-bound value and try to repair it afterwards.

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
* **`Pool`** — one contiguous block from a resource, with two phases. **Build**: `reserve()`
  bump-allocates and may grow (moving the base). **Query**: `freeze()` makes the base stable and
  enables `mirror()`, which copies a whole frozen block into another resource — the host-to-device
  upload, one `memcpy`, no patching. Move-only, host-only; never pass a `Pool` to a kernel.
* **`PODVector` / `PODSpan`** — array descriptors holding a byte offset plus size and capacity.
  Trivially copyable, resolved against a base supplied per call (`at(base, i)`) or bound once
  (`bind(base)`).

Classes built on this expose two accessor families — an explicit-base overload valid mid-build, and
a no-argument bound overload for a finished structure — plus `boundView()` where a bound
`const Mesh&` is needed before the real object can be bound. `DCEL::MeshT` is the reference
implementation of the pattern.

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

## What is not

| Component | Blocker |
|---|---|
| `TreeBVH` / `PackedBVH` | `std::vector` storage; `SharedPtrStorage` primitives; the scalar `pruneTraverse` path delegates to a `std::function`-based `traverse()` on a heap stack |
| `MeshSDF` / `FlatMeshSDF` / `TriMeshSDF` | Blocked on the BVH |
| `Triangle<T, Meta>` (AoS), `Octree` | Not started |
| `PointCloudBVH`, `PointCloudHashGrid`, `SFC` | Not started; the point-cloud BVH additionally has to *build* on device |
| `ImplicitFunction`, `CSG`, `Transform`, analytic SDFs | Still the original virtual-`value()` design; this is where the tape returns |
| Parsers (`OBJ`/`PLY`/`STL`/`VTK`/`Soup`), `Random`, `SimpleTimer` | Host-only by design — no port intended |

## Roadmap

1. **DCEL reconcile chain.** `FaceT::computeCentroid`/`computeNormal`/`computeArea` rewritten as
   streaming half-edge walks (they currently open with `gatherVertexIndices()`, which materializes a
   `std::vector`), then device-resident CSR vertex→face adjacency, then
   `VertexT::computeVertexNormalAngleWeighted`. All three parts or none: the angle-weighted
   pseudonormal is what makes the sign correct, so a device `reconcile()` covering only faces would
   leave signs silently wrong near vertices and edges.
2. **BVH.** Give `pruneTraverse` a real scalar implementation (fixed stack, hand-rolled sort over
   the ≤K children); move `PackedBVH`'s three arrays onto `Pool`/`PODVector`; add a trivially-copyable
   view plus a `[gpu]` test; then `TriMeshSDF` and `MeshSDF`. `TreeBVH` stays host-only — it is the
   builder, and static geometry builds on the host. Open decisions: whether to drop
   `SharedPtrStorage` entirely, and the device stack depth (`[256]` is 4 KB/thread for `double`,
   occupancy-hostile; 64 covers a million primitives at K=4).
3. **Point clouds.** Device-side `PointCloudBVH` build (Morton codes + radix sort) and a
   `PointCloudHashGrid` counterpart. Independent of 1–2.
4. **Analytic SDF / transform / combiner traits.** Each primitive's formula moves into a trait as a
   `static EBGEOMETRY_HOST_DEVICE eval()`, with the existing virtual `value()` becoming a thin
   delegate. **This has no dependency on the memory work** — the parameters are a handful of plain
   values, there is nothing to place in a `Pool` — so it can run in parallel with steps 1–3 rather
   than queuing behind them. It is worth doing separately and first because the tape's interpreter
   switch is precisely a dispatcher over these trait statics, and because a kernel can call
   `eval()` directly, giving the formulas real device coverage before any tape exists. The
   exceptions are `BVHUnionIF`/`BVHSmoothUnionIF`, which wrap a `PackedBVH` and therefore wait for
   step 2.
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
   other device-callable calls; `EBGEOMETRY_HOST` for anything touching a `Pool`, `std::vector`,
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
cmake --preset cuda -DCMAKE_CUDA_ARCHITECTURES=<arch>   # e.g. 86 for an RTX 3080 Ti; preset default is 70
cmake --build build/cuda --target EBGeometry_GPUDeviceTests --parallel $(nproc)
cd build/cuda && ctest -L gpu-device --output-on-failure
```

CI compiles both backends on every push but has no physical GPU, so the device-side assertions only
ever execute on a developer machine. Running the above before pushing GPU work is therefore not
optional — a green CI GPU lane means "it compiles", nothing more.
