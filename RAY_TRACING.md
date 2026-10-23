<!--
SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>

SPDX-License-Identifier: GPL-3.0-or-later
-->

# Ray tracing: development document

> **To be deleted before merge.** This is the live working document for the `ray_tracing` branch and
> its pull request: scope, decisions with their reasons, open questions, and progress. What survives
> the work goes into the Sphinx docs (Concepts and Implem pages), the Doxygen comments and an example;
> this file does not.

## Goal

Ray queries as a library feature in their own right, not a part of the renderer (#93): does a ray hit
a scene, where first, and does the path of a particle from `x0` to `x1` cross the surface. They work
on every shape the library can evaluate (analytic shapes, mesh SDFs, BVH unions, tapes, user shapes),
on the host and on a device, one query per thread or a packet of queries per thread. The renderer is
rebuilt on top of them later.

Approach (the maintainer's): reuse the BVH traversals; ray trace exactly where a shape has a ray form,
and sphere trace where it does not. The same pattern as `signedDistance()`: a free function for any
shape, with members on the Tape (and later the fused CSG nodes).

## Where this comes from

The renderer stack (#214 core, #215 examples, #216 ray tape; drafts, off `dev` at c548641) built most
of the ray machinery inside the renderer work. This branch lifts the standalone part out of it, fixes
what the review below found, and lands it first. #214–#216 are then rebuilt on top (see *Sequence*).

### What #214/#216 already have, and what is reused

| Piece | Where | Reuse |
|---|---|---|
| `RayT` (origin, unit direction, `at(t)`) | `EBGeometry_Ray.hpp` | as is |
| `HitT`: status, t, point, primitive id, normal, entering/leaving, exact | `EBGeometry_Ray.hpp` | as is |
| `HitStatus` three-state: Hit / Miss / Unresolved (never report "gave up" as a miss) | `EBGeometry_Ray.hpp` | as is (decision D2) |
| `RayOptions`: tolerance and step relative to the box diagonal, step budget | `EBGeometry_Ray.hpp` | as is |
| `RayForm<S>`: opt-in specialisable trait, `available` + static `firstHit()` (+ `span()`) | `EBGeometry_Ray.hpp` | as is |
| `firstHit(shape, ray, tMin, tMax, options)`, `intersects(shape, a, b, options)`: any `isShape`, device-callable | `EBGeometry_Ray.hpp` | extend (segments, packets) |
| `RayDetail::march()`: sphere trace inside the box for Distance/Bound, fixed steps + bisection for NotADistance; Unresolved when out of steps | `EBGeometry_RayImplem.hpp` | as is, plus normals |
| Closed-form ray forms + `span()`: plane, sphere, box, cylinder, infinite cylinder, capsule | `EBGeometry_RayImplem.hpp` | as is |
| `RayForm<TriMeshSDF>`: `PackedBVH::traverse()` keyed by box entry distance, nearest child first, pruned beyond the best hit; `TriangleSoA::rayHit()` at the leaves (Möller–Trumbore) | `EBGeometry_RayImplem.hpp`, `EBGeometry_TriangleSoA*.hpp` | reuse the traversal; rewrite the leaf kernel (R1) |
| `RaySpan`/`RaySpans`, `spansOf()`: where a ray is inside an object, for CSG | `EBGeometry_Ray*.hpp` (#216) | PR B |
| Tape leaf ids, `primitiveAt()`, `Tape::firstHit()` following clauses, `Op::RayMark` | `EBGeometry_Tape*.hpp` (#214/#216) | PR B (after #224) |

### Review findings to fix here

- **R1. The triangle ray test is a scalar loop over the W lanes.** Write it once against `V` (a scalar
  or `SIMD::Pack`), as #197 did for the distance kernel: SIMD lanes on the host, scalar on a device.
- **R2. The BVH ray test builds each node's box separately** (`getBoundingVolume(node)` per child). A
  K-children slab test reading the parent's SoA child-box row in one pass, as `pruneTraverse()` does
  for distances, is the expected fast path. Measure first; the probes decide.
- **R3. Missing ray forms on the BVH shapes:** `MeshSDF` and `FlatMeshSDF` (the default mesh SDFs:
  planar convex polygon faces, ray–plane plus point-in-polygon) and `BVHUnion<T, P, K>` (walk the
  union's BVH with the ray; at a leaf call `firstHit()` on the primitive, which is exact where `P` has a
  ray form and marches inside the primitive's box where it does not). This is the "reuse the BVH
  traversals, ray intersection where we can, sphere tracing where we can't" rule in one place.
- **R4. A path from `x0` to `x1` returns only a status.** Particles need where, which face, which
  normal: a segment overload of `firstHit()` returning `HitT`. A yes/no query (`intersects()`) can stop
  at the first crossing found instead of the nearest (any-hit early exit).
- **R5. Marched hits carry no normal.** Fill it from the gradient where the shape has no ray form
  (D9).
- **R6. Lambdas into device code.** `march()` and the traversals pass lambdas to device code, the
  pattern nvcc 12.5 miscompiled in #217. Every ray form, the march and the packet path get a `[gpu]`
  test against the host (#214 had one).

## Decisions

| # | Decision | Reason |
|---|---|---|
| D1 | **Batched queries are packets sharing a walk.** One thread traces a fixed packet of N rays/segments through the BVH together: a node is visited if any active ray in the packet needs it, and the leaf tests are vectorised across the packet. Alongside the point-like kernel (one query per thread). | Coherent queries (particles in one cell, a tile of camera rays) share most of their traversal; per-ray work is the fallback as the packet diverges. |
| D2 | **`intersects()` is three-state** (`HitStatus`: Hit, Miss, Unresolved). `firstHit(shape, x0, x1)` returns the full `HitT`. | A particle must not leak through a wall because a grazing march gave up and called it a miss. |
| D3 | **The Tape is marched in this PR**; exact clause-by-clause ray tracing comes after #224. `Tape::firstHit()`/`Tape::intersects()` exist now as members. | Marching is always correct and touches nothing inside the Tape, while #224 rewrites its operations. |
| D4 | **First crossing only in this PR.** Every crossing along a path (`forEachHit`) comes with the span machinery in PR B. | Enough to stop a particle at a wall; the in/out sequence needs spans. |
| D5 | **Every shape takes a packet.** BVH shapes (mesh SDFs, `BVHUnion`) walk it together; closed-form and marched shapes loop over its rays (a generic default). | Callers never need to know which shapes walk and which loop. |
| D6 | **Packet size is a template parameter:** `RayPacket<T, N>`, N fixed at compile time, with a mask of active rays (a packet can be partly filled). Trivially copyable, fixed arrays. 4 and 8 tested and measured. | Registers on a device; the caller matches N to the coherence of its queries. |
| D7 | **Packet layout is SoA:** origins and directions as x[N], y[N], z[N], per-ray tMin/tMax, like `TriangleSoA`. | The slab test and the triangle kernel load N rays into `SIMD::Pack` lanes directly on the host; on a device the same struct loops. |
| D8 | **A packet query returns `Array<HitT<T>, N>`**, one ordinary `HitT` per ray, inactive lanes left as Miss. | Same result type as a single query. |
| D9 | **Marched hits always carry a normal:** central differences of the signed distance at the hit (6 evaluations, only on a hit). | Particle reflection and shading need it. |
| D10 | **No new closed forms in this PR** (torus, cones, rounded box/cylinder march). | They march correctly; closed forms when a probe shows they matter (the torus quartic is fragile in float). |
| D11 | **No library launchers.** Users call `firstHit()`/`intersects()` (single or packet) in their own `GPU::transform` functor or host loop, as with `signedDistance()`; the example and the probes show both patterns. | A small API, the same pattern as every other query. |
| D12 | **Headers: core + per component.** `EBGeometry_Ray.hpp` holds `RayT`, `HitT`, `HitStatus`, `RayOptions`, `RayForm`, `RayPacket`, the march and the free functions; each component's ray form sits next to that component (mesh SDFs, `BVHUnion`, the triangle groups, the analytic shapes). | Device code includes only what it uses; the core includes no mesh headers. |
| D13 | **Segment queries measure t as distance from `x0`** (0 to \|x1 − x0\|): a segment is a ray from `x0` with tMax = \|x1 − x0\|. | One unit for rays and segments. |
| D14 | **A crossing at t = 0 is a hit; a caller stepping off a surface passes a small tMin** (the segment overload takes one too). No default start offset. | Correct for a particle that is on the wall; the example shows the step-off. |
| D15 | **Example: a new `Examples/ParticlePaths`**: particles stepping through a mesh, each step a segment query reporting crossing point, face and normal; one query per thread and packets; a device section through `GPU::transform`; its own Sphinx page. | Shows the particle use case #93 asks for. |

## Open questions

None at present. (Q1–Q7 of the first draft were settled in the interview on 2026-10-23 as D5–D15.)

## Scope of this PR (PR A)

- Core types and free functions (`firstHit`, `intersects`) for rays and for segments; any-hit early
  exit for `intersects`.
- The march fallback, with normals (D9).
- Closed-form ray forms: plane, sphere, box, cylinder, infinite cylinder, capsule.
- BVH ray forms reusing `PackedBVH::traverse()`: `TriMeshSDF`, `MeshSDF`, `FlatMeshSDF`, `BVHUnion`.
- The triangle ray kernel written once against `SIMD::Pack` (R1).
- Packets (D1, D5–D8): `RayPacket<T, N>` (SoA, active mask), a shared BVH walk for the BVH shapes, a per-ray loop for the rest, results as `Array<HitT<T>, N>`.
- `Tape::firstHit()`, `Tape::intersects()` by marching (D3).
- Per CLAUDE.md: unit tests (`TEMPLATE_TEST_CASE`, independent references: closed forms, brute force
  over all faces), `[gpu]` cases for every ray form, the march and packets; `Tests/InstantiateAll.cpp`;
  performance cases (one query per thread and packets, host and `hostdevice` pairs: mesh, BVH union,
  analytic shape, tape march); Sphinx Concepts + Implem pages; `Examples/ParticlePaths` with its page (D15);
  the receipt.

Not in this PR: exact CSG/Tape ray tracing, every crossing along a path, the renderer.

## Sequence

1. **PR A (this one):** ray queries, off `dev`.
2. **PR B, after #224 merges:** ray forms for the fused CSG nodes (union, intersection and difference
   combine their operands' spans; affine nodes map the ray), and the Tape following the same span maths
   clause by clause (#216's ray evaluator, `Op::RayMark` around subtrees that must be marched), sharing
   one implementation with the fused nodes as `CSGOps` does for values. Tape leaf ids. `forEachHit`.
3. **PR C:** the renderer, viewer and examples (#214–#216) rebuilt on A and B, generic over
   `firstHit()`; #214–#216 closed as superseded, their branches kept.

## Progress

- [x] Branch `ray_tracing` off `dev` (8e03593), draft PR, this document.
- [x] Interview: D1–D15 (2026-10-23).
- [ ] Core types and free functions lifted from #214/#216.
- [ ] Segment overloads, any-hit `intersects`.
- [ ] March normals.
- [ ] Triangle ray kernel on `SIMD::Pack`.
- [ ] Ray forms: `MeshSDF`/`FlatMeshSDF`, `BVHUnion`.
- [ ] Packets.
- [ ] `Tape::firstHit()`/`intersects()`.
- [ ] Tests (host, `[gpu]`), `InstantiateAll`.
- [ ] Performance cases; receipt (host + CUDA).
- [ ] Sphinx pages, Doxygen, example.
- [ ] Delete this file.
