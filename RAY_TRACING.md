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
  (open question Q3).
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

## Open questions (interview in progress)

- **Q1. Packet scope.** Every shape takes a packet (BVH shapes walk it together; closed-form and
  marched shapes loop over its rays), or only the BVH shapes? *Proposed: every shape.*
- **Q2. Packet size.** A template parameter (`RayPacket<T, N>`, with an active mask, trivially
  copyable, registers on a device) or one width fixed by the library per precision? *Proposed:
  template parameter, 4 and 8 tested and measured.*
- **Q3. Normals for marched hits.** From the gradient by central differences on every hit (6
  evaluations, only on a hit), only when asked through `RayOptions`, or never? *Proposed: always.*
- **Q4. Closed forms for torus, cones, rounded box and cylinder.** Now, or when a probe shows they
  matter (they march correctly; the torus needs a quartic, fragile in float)? *Proposed: later.*
- **Q5. Batched host/device launchers.** Besides calling `firstHit()` inside a user's own
  `GPU::transform` functor, should the library offer a launcher over arrays of rays/segments (and
  packets)?
- **Q6. Header layout.** One `EBGeometry_Ray.hpp` with every specialisation, or the core types there
  and each component's ray form next to the component (so device code includes only what it needs)?
- **Q7. Example.** A new `Examples/ParticlePaths` (particles stepping through a mesh, reporting
  crossing point, face and normal; device section through `GPU::transform`) or extend an existing one?

## Scope of this PR (PR A)

- Core types and free functions (`firstHit`, `intersects`) for rays and for segments; any-hit early
  exit for `intersects`.
- The march fallback, with normals (per Q3).
- Closed-form ray forms: plane, sphere, box, cylinder, infinite cylinder, capsule.
- BVH ray forms reusing `PackedBVH::traverse()`: `TriMeshSDF`, `MeshSDF`, `FlatMeshSDF`, `BVHUnion`.
- The triangle ray kernel written once against `SIMD::Pack` (R1).
- Packets (D1): `RayPacket`, a shared BVH walk, per-ray fallback for the rest.
- `Tape::firstHit()`, `Tape::intersects()` by marching (D3).
- Per CLAUDE.md: unit tests (`TEMPLATE_TEST_CASE`, independent references: closed forms, brute force
  over all faces), `[gpu]` cases for every ray form, the march and packets; `Tests/InstantiateAll.cpp`;
  performance cases (one query per thread and packets, host and `hostdevice` pairs: mesh, BVH union,
  analytic shape, tape march); Sphinx Concepts + Implem pages; an example with its page (Q7); the
  receipt.

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
- [ ] Interview: Q1–Q7.
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
