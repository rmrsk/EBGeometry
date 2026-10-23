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
on the host and on a device, one query per call (one per thread on a device). The renderer is
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
| `firstHit(shape, ray, tMin, tMax, options)`, `intersects(shape, a, b, options)`: any `isShape`, device-callable | `EBGeometry_Ray.hpp` | extend (segments) |
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
  pattern nvcc 12.5 miscompiled in #217. Every ray form and the march get a `[gpu]`
  test against the host (#214 had one).

## Decisions

| # | Decision | Reason |
|---|---|---|
| D1 | ~~Batched queries are packets sharing a walk~~ **superseded by D20 (no packets).** | |
| D2 | **`intersects()` is three-state** (`HitStatus`: Hit, Miss, Unresolved). `firstHit(shape, x0, x1)` returns the full `HitT`. | A particle must not leak through a wall because a grazing march gave up and called it a miss. |
| D3 | **The Tape is marched in this PR**; exact clause-by-clause ray tracing comes after #224. `Tape::firstHit()`/`Tape::intersects()` exist now as members. | Marching is always correct and touches nothing inside the Tape, while #224 rewrites its operations. |
| D4 | **First crossing only in this PR.** Every crossing along a path (`forEachHit`) comes with the span machinery in PR B. | Enough to stop a particle at a wall; the in/out sequence needs spans. |
| D5 | ~~(packets)~~ **superseded by D20.** | |
| D6 | ~~(packets)~~ **superseded by D20.** | |
| D7 | ~~(packets)~~ **superseded by D20.** | |
| D8 | ~~(packets)~~ **superseded by D20.** | |
| D9 | **Marched hits always carry a normal:** central differences of the signed distance at the hit (6 evaluations, only on a hit). | Particle reflection and shading need it. |
| D10 | **No new closed forms in this PR** (torus, cones, rounded box/cylinder march). | They march correctly; closed forms when a probe shows they matter (the torus quartic is fragile in float). |
| D11 | **No library launchers.** Users call `firstHit()`/`intersects()` in their own `GPU::transform` functor or host loop, as with `signedDistance()`; the example and the probes show both patterns. | A small API, the same pattern as every other query. |
| D12 | **Headers: core + per component.** `EBGeometry_Ray.hpp` holds `RayT`, `HitT`, `HitStatus`, `RayOptions`, `RayForm`, the march and the free functions; each component's ray form sits next to that component (mesh SDFs, `BVHUnion`, the triangle groups, the analytic shapes). | Device code includes only what it uses; the core includes no mesh headers. |
| D13 | **Segment queries measure t as distance from `x0`** (0 to \|x1 − x0\|): a segment is a ray from `x0` with tMax = \|x1 − x0\|. | One unit for rays and segments. |
| D14 | **A crossing at t = 0 is a hit; a caller stepping off a surface passes a small tMin** (the segment overload takes one too). No default start offset. | Correct for a particle that is on the wall; the example shows the step-off. |
| D15 | ~~A new `Examples/ParticlePaths`~~ superseded by D17. | |
| D16 | **Builds on #230's general traversal** (see *Depends on #230*). | Ray casts are a main use case; the walk is designed and measured once, for every query type. |
| D17 | **One example, `Examples/RayTracing`, on a `TriMeshSDF`, in two parts:** the user-facing `firstHit()`/`intersects()`, then the same hits from a hand-written `traverse()` with a `RayQuery` and the user's own leaf evaluator. | Shows the API, and how to walk the BVH yourself when the API does not fit. |
| D18 | **One performance probe for ray tracing a `TriMeshSDF`:** the user-facing API (one ray per thread) and a hand-rolled walk (`traverse()` with a `RayQuery` and the probe's own leaf evaluator), host and `hostdevice`, coherent and random rays. | Shows whether the API costs anything over hand-written code. |
| D19 | **Unit tests check known cases against an AABB unit cube** (a 12-triangle `TriMeshSDF` against the exact slab answer): axis-aligned and diagonal rays, through an edge and a corner, from inside, grazing in a face's plane, misses, segments stopping short, a crossing at t = 0. | Independent closed-form reference that exercises the edge and corner robustness. |
| D20 | **No packets** (several rays per thread sharing one walk). One query per call; many queries are the user's own loop, or one per thread in their own functor. #230 leaves room to add packets later without breaking the API. | The SIMD is already used across a node's K child boxes and a leaf's W triangles; packets only amortise node fetches for coherent rays, which particles and one-ray-per-GPU-thread are not, at the cost of lane masks in the walk and every callback. Decided 2026-10-23. |

## Depends on #230 (BVH traversal)

The ray queries build on a rewritten, general `PackedBVH::traverse()` that lands first in #230 (see
`BVH_TRAVERSAL.md` on the `bvh_traversal` branch): one walk for any query type with a SIMD K-children
test (`PointQuery`, `RayQuery`, or the user's own), order policies, a state object, a leaf evaluator
and a bound. `pruneTraverse()` and the old `traverse()` go. Consequences here:

- `RayQuery` (the walk's ray: origin, inverse direction, tMin, slab test) lives in #230; this PR adds
  `RayT`, `HitT`, `RayOptions`, `RayForm`, `firstHit`/`intersects` and the ray forms on top of it.
- Every BVH ray form (`TriMeshSDF`, `MeshSDF`, `FlatMeshSDF`, `BVHUnion`) is `traverse()` with a
  `RayQuery` and a leaf evaluator that tests the leaf's primitives; no `rayTraverse()`. Section 1 of
  *Traversal algorithms* below describes what that call does; the walk itself is #230's.
- This branch is rebased on #230 once it lands, before any code goes in.

## Interface (proposed, for review before implementation)

Declarations only; every function is `EBGEOMETRY_HOST_DEVICE`, `noexcept` and `[[nodiscard]]` unless
said otherwise, in `namespace EBGeometry`. Comments give the contract.

### Core types (`EBGeometry_Ray.hpp`)

```cpp
// The points origin + t * direction. Queries take the direction to be of unit length, so t is a
// distance along the ray.
template <class T>
struct RayT
{
  using Scalar = T;

  Vec3T<T> m_origin;
  Vec3T<T> m_direction;

  Vec3T<T> at(T a_t) const;
};

enum class HitStatus : uint8_t
{
  Miss,      // no crossing in the interval
  Hit,       // a crossing; HitT says where
  Unresolved // the query ran out of steps before it could decide; never reported as a miss
};

// Where a ray first crosses a surface, entering or leaving.
template <class T>
struct HitT
{
  HitStatus m_status    = HitStatus::Miss;
  T         m_t         = Math::Limits<T>::infinity(); // crossing (Hit), last point reached (Unresolved)
  Vec3T<T>  m_point     = Vec3T<T>::zeros();           // the point at m_t
  uint32_t  m_primitive = 0;     // finest element the shape names there: a mesh's face id, a union's
                                 // operand index, 0 otherwise
  Vec3T<T>  m_normal    = Vec3T<T>::zeros(); // outward unit normal: from the ray form, or the gradient of
                                             // the signed distance for a marched hit (D9)
  bool      m_entering  = false; // the ray goes from outside to inside there
  bool      m_exact     = false; // found in closed form, not to within RayOptions::m_tolerance

  bool hit() const; // m_status == HitStatus::Hit
};

// How a query marches where a shape has no ray form. Lengths are relative to the shape's scale: the
// diagonal of its bounding box, or m_scale if positive (1 for an unbounded box).
template <class T>
struct RayOptions
{
  T        m_tolerance = T(1e-4);  // a crossing is accepted where |value| < this * scale
  T        m_scale     = T(0);     // 0: the box diagonal
  uint32_t m_maxSteps  = 512;      // steps before a query is Unresolved
  T        m_step      = T(0.005); // fixed step * scale where the value is not a distance
};
```

### Queries (free functions, any shape `S` with `isShape<S>`)

```cpp
// First crossing in [a_tMin, a_tMax], entering or leaving.
template <class T, class S>
HitT<T> firstHit(const S& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax,
                 const RayOptions<T>& a_options = RayOptions<T>());

// First crossing on the segment from a_from to a_to; m_t is the distance from a_from (D13).
// A crossing at a_tMin is a hit; pass a small a_tMin to step off a surface (D14).
template <class T, class S>
HitT<T> firstHit(const S& a_shape, const Vec3T<T>& a_from, const Vec3T<T>& a_to, T a_tMin = T(0),
                 const RayOptions<T>& a_options = RayOptions<T>());

// Whether there is any crossing (D2): stops at the first crossing found, not the nearest.
template <class T, class S>
HitStatus intersects(const S& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax,
                     const RayOptions<T>& a_options = RayOptions<T>());

template <class T, class S>
HitStatus intersects(const S& a_shape, const Vec3T<T>& a_from, const Vec3T<T>& a_to, T a_tMin = T(0),
                     const RayOptions<T>& a_options = RayOptions<T>());
```

Use, one query per thread, in the caller's own functor (D11):

```cpp
struct PathQuery // a particle's step from x0 to x1
{
  TriMeshSDF<T, K, W> m_mesh; // a rebasedView() on a device

  EBGEOMETRY_HOST_DEVICE HitT<T> operator()(const Segment& a_s) const noexcept
  {
    return firstHit(m_mesh, a_s.from, a_s.to);
  }
};

GPU::transform(PathQuery{view}, segments, hits); // host loop in a host build
```

### Usage: a list of particle steps, start and stop positions

A user holds the steps in their own type. The queries take `Vec3T<T>`; the conversion is one line in
the functor, so the user's type needs no traits or adaptors.

```cpp
#include "EBGeometry.hpp"

using namespace EBGeometry;
using T    = double;
using Wall = TriMeshSDF<T, BVH::DefaultBranchingRatio<T>(), TriangleSoA::DefaultWidth<T>()>;

// The user's own type: one particle step from x0 to x1.
struct Step
{
  double x0[3];
  double x1[3];
};

// What the user wants back for each step.
struct Crossing
{
  int      status; // 0 miss, 1 hit, 2 unresolved
  double   t;      // distance from x0 to the wall
  double   point[3];
  double   normal[3];
  uint32_t face;
};

// --- One step per thread -------------------------------------------------------------------
struct StepQuery
{
  Wall m_wall; // a rebasedView() of the mesh: valid on the device

  EBGEOMETRY_HOST_DEVICE Crossing
  operator()(const Step& a_step) const noexcept
  {
    const Vec3T<T> x0(a_step.x0[0], a_step.x0[1], a_step.x0[2]);
    const Vec3T<T> x1(a_step.x1[0], a_step.x1[1], a_step.x1[2]);

    const HitT<T> hit = firstHit(m_wall, x0, x1); // first crossing on the segment, or a miss

    Crossing c{};
    c.status = static_cast<int>(hit.m_status);
    c.t      = hit.m_t;
    c.face   = hit.m_primitive;

    for (int d = 0; d < 3; d++) {
      c.point[d]  = hit.m_point[d];
      c.normal[d] = hit.m_normal[d];
    }

    return c;
  }
};

int
main()
{
  // The wall: a mesh read into a pool, frozen and mirrored to the device.
  Pool pool(hostMemoryResource());

  const Wall wall = Parser::readIntoTriMeshSDF<T>("wall.stl", pool, BVH::Construction::SAH);

  pool.freeze();

  const Pool devicePool = Pool::mirror(pool, GPU::memoryResource());
  const auto view       = wall.rebasedView(devicePool);

  std::vector<Step> steps = /* the user's list */;

  // One step per thread: GPU::transform copies the steps over, runs StepQuery on each, and copies
  //    the results back. In a host build the same call is a host loop.
  std::vector<Crossing> crossings(steps.size());

  GPU::transform(StepQuery{view}, steps, crossings);

  // On the host, no device: the same functions on the host object.
  for (const Step& s : steps) {
    const HitStatus st = intersects(wall, Vec3T<T>(s.x0[0], s.x0[1], s.x0[2]), Vec3T<T>(s.x1[0], s.x1[1], s.x1[2]));
    // ...
  }
}
```

A particle that hit the wall and is reflected starts its next step on the surface; it passes a small
`tMin` so the crossing it is sitting on is not found again (D14):

```cpp
const HitT<T> next = firstHit(m_wall, x0, x1, T(1e-9) * diagonal);
```

The same works with a user's own vector type (with `operator[]`, or `.x/.y/.z`): only the two
conversion lines change. With `std::vector<Vec3T<T>>` start and stop lists, the functor indexes them
through `GPU::parallelFor` instead of taking a `Step`. Steps in spatial (Morton) order keep
neighbouring threads on similar paths through the BVH.

### Traversal algorithms

#### 1. One ray through a BVH (`TriMeshSDF`, `MeshSDF`, `BVHUnion`)

`PackedBVH::traverse()` from #230 with a `RayQuery`; only the state and the leaf evaluator are
ray-specific. `best` is the nearest crossing found so far (initially the query's `tMax`).

```text
query   = RayQuery{origin, direction, tMin}  : key of a child box = where the ray enters it (slab test,
                                               all K children in one SIMD pass), +inf if it misses
bound   = best                               : a box entered beyond the best crossing is skipped
order   = NearestFirst                       : a crossing found early prunes the boxes behind it
leaf    = for each primitive: crossing = rayTest(prim, ray, tMin, best)
          if crossing at t < best:  best = t, keep face id, normal, entering

traverse(query, state, leaf, bound)          -> HitT from the state (Miss if nothing below tMax)
```

- **Slab test** (in `RayQuery`). `t0 = (lo - o) * invD`, `t1 = (hi - o) * invD` per axis, entry = max
  of the per-axis minima and `tMin`, exit = min of the maxima; hit iff entry ≤ exit. Robust form: a zero
  direction component gives ±inf, and a `0 * inf` NaN is dropped by ordering the min/max so NaN loses.
  Boxes are padded by a few ulps of their extent so a ray through a shared face or edge cannot slip
  between two neighbouring boxes.
- **The bound is re-read at every node**, so a crossing found in an earlier sibling's subtree prunes the
  later ones (nearest-first order makes this the common case).
- **Leaf tests (`rayTest`)**, exact:
  - `TriMeshSDF`: Möller–Trumbore on a group of W triangles, written once against `V` (R1): on the host
    `V = SIMD::Pack<T, W>`, all W triangles in one pass, the nearest lane with `t ∈ [tMin, best)`
    kept; on a device `V = T`, one triangle at a time. A small barycentric tolerance keeps rays through
    shared edges from slipping between triangles. Entering = `dot(direction, face normal) < 0`.
  - `MeshSDF`/`FlatMeshSDF`: ray–plane with the face's plane, then point-in-convex-polygon on the DCEL
    face (edge-side tests). `FlatMeshSDF` has no BVH: every face is tested.
  - `BVHUnion<T, P, K>`: `firstHit(operand, ray, max(tMin, boxEntry), min(best, boxExit))`: the
    operand's own ray form if `P` has one, otherwise the march (2) clipped to the operand's box. This
    is the "ray intersection where we can, sphere tracing where we can't" rule.
- **Any-hit (`intersects`)**: the same walk with `AsStored` order (no sort), and the bound dropping to
  -infinity at the first crossing, which ends the walk.

#### 2. The march (shapes without a ray form, `Tape` in this PR)

From #214's `RayDetail::march()`, unchanged except the normal:

```text
clip [tMin, tMax] to the shape's box (padded by eps)          -> [t0, t1], or Miss
eps = tolerance * scale;  v0 = f(ray(t0));  inside = v0 < 0
if |v0| < eps: Hit at t0
if quality is Distance or Bound:                               # sphere tracing
    repeat up to maxSteps:
        v = f(ray(t))
        if sign(v) != sign(v0): Hit at bisect(tPrev, t)        # stepped across: refine
        if |v| < eps:           Hit at t
        tPrev = t;  t += |v|                                   # never steps over the surface
        if t > t1:              Miss
else (NotADistance):                                           # no step bound: fixed steps
    repeat up to maxSteps:  t += step*scale;  on a sign change: Hit at bisect(tPrev, t);  t >= t1: Miss
Unresolved (with the last t)                                   # out of steps: never a Miss
on a Hit: normal = normalize(central differences of f at the point, h = eps)   (D9)
          entering = !inside;  primitive = the shape's closest face / leaf if it names one
```

- Starting inside works the same way, stepping by |v| to the exit.
- `Bound` (a lower bound on the distance, e.g. CSG intersections) is safe to sphere trace, just slower;
  a ray grazing a surface takes many small steps and may end Unresolved, which callers must handle.

#### 3. Segments

`firstHit(shape, x0, x1, tMin)` is the ray `x0 + t d`, `d = (x1 − x0)/|x1 − x0|`, over
`[tMin, |x1 − x0|]` (D13). `intersects(shape, x0, x1)` first compares the signs of the value at the
two ends: different signs mean a crossing (Hit) without a walk; the same sign runs the any-hit walk
(the segment may go in and out again). A zero-length segment is a Miss.

### Tape members (D3)

The free functions cover every shape; the Tape also has them as members, as it has
`signedDistance()`. In this PR they march; after #224 they follow the clauses exactly.

```cpp
template <class T, class... UserPrimitives>
class Tape
{
  // ...
  HitT<T>   firstHit(const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& = RayOptions<T>()) const;
  HitT<T>   firstHit(const Vec3T<T>& a_from, const Vec3T<T>& a_to, T a_tMin = T(0),
                     const RayOptions<T>& = RayOptions<T>()) const;
  HitStatus intersects(const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& = RayOptions<T>()) const;
  HitStatus intersects(const Vec3T<T>& a_from, const Vec3T<T>& a_to, T a_tMin = T(0),
                       const RayOptions<T>& = RayOptions<T>()) const;
};
```

### Extension point: `RayForm<S>`

A shape without a specialisation is marched. A specialisation makes it ray traced; only `firstHit` is
required; `intersects` is optional, with a generic default.

```cpp
template <class S, class = void>
struct RayForm
{
  static constexpr bool available = false;
};

template <>
struct RayForm<MyShape>
{
  static constexpr bool available = true;

  // Required: first crossing in [a_tMin, a_tMax], every HitT field filled.
  static HitT<T> firstHit(const MyShape&, const RayT<T>&, T a_tMin, T a_tMax, const RayOptions<T>&);

  // Optional: any crossing, early exit (default: firstHit(...).m_status).
  static HitStatus intersects(const MyShape&, const RayT<T>&, T a_tMin, T a_tMax, const RayOptions<T>&);
};
```

Ray forms in this PR, each next to its component (D12):

| Shape | Ray form | Header |
|---|---|---|
| `PlaneSDF`, `SphereSDF`, `BoxSDF`, `CylinderSDF`, `InfiniteCylinderSDF`, `CapsuleSDF` | closed form | `EBGeometry_AnalyticRayForms.hpp` |
| `TriMeshSDF` | BVH walk (`PackedBVH::traverse()`), triangle kernel on `SIMD::Pack` at the leaves | `EBGeometry_MeshRayForms.hpp` |
| `MeshSDF`, `FlatMeshSDF` | BVH walk / all faces, ray–plane + point-in-convex-polygon on the DCEL faces | `EBGeometry_MeshRayForms.hpp` |
| `BVHUnion<T, P, K>` | BVH walk; at a leaf `firstHit()` on the operand (its ray form, or marched in its box) | `EBGeometry_BVHUnion.hpp` |
| `Tape` | marched (exact after #224) | `EBGeometry_Tape.hpp` (members) |
| everything else | marched | — |

### Points to confirm

- **Members on other shapes.** Only the Tape gets member functions; every other shape uses the free
  functions. Mesh SDFs and `BVHUnion` could get members too, for symmetry.
- **`m_primitive` for `BVHUnion`**: the operand's index in the array the union was built from (the
  union's leaf order is internal).
- **Names**: `firstHit`/`intersects` as in #214; `RayT`/`HitT` (the `...T` suffix follows
  `Vec3T`/`AABBT`).

## Open questions

- **Tape, beyond marching.** Proposed (not decided): the Tape compiler stores one world-frame box per
  leaf, grown by every enclosing operation that moves the surface (Offset, blends, Blur, Mollify) and
  mapped through every enclosing transform. For min/max CSG the zero set lies inside the union of those
  boxes, so `firstHit()` marches only where the ray is inside one of them (empty-space skipping), and a
  user can read the boxes and build their own BVH over them. Unbounded leaves (planes, complements)
  have infinite boxes and fall back to marching the whole interval. Touches the Tape compiler: after
  #229 (fused CSG) lands. Exact clause-by-clause span tracing (#216) dropped from the plan unless a
  probe calls for it.
- **The Tape's automatic union BVH.** The maintainer is not convinced the Tape should build its own BVH
  for large unions. Evidence (#224, #226): on the GPU in float the automatic BVH is slower than none
  for `union64` (1.75 vs 1.56 ns), a typed `BVHUnion` as one primitive beats it on both sides (1.01 ns
  GPU, 88 vs 97 ns CPU), and `evaluateUnion` is one of the heavy paths that set every tape kernel's
  register budget. Without it, large unions are written as explicit `BVHUnion` primitives (or #224's
  `AnyOf` for mixed types); CPU users of `Union(many shapes)` lose ~1.6x unless they switch. To be
  raised on #224.
- **`MeshSDF`/`FlatMeshSDF`/`BVHUnion` ray forms**: in this PR or a follow-up.

## Scope of this PR (PR A)

- Core types and free functions (`firstHit`, `intersects`) for rays and for segments; any-hit early
  exit for `intersects`.
- The march fallback, with normals (D9).
- Closed-form ray forms: plane, sphere, box, cylinder, infinite cylinder, capsule.
- BVH ray forms reusing `PackedBVH::traverse()`: `TriMeshSDF`, `MeshSDF`, `FlatMeshSDF`, `BVHUnion`.
- The triangle ray kernel written once against `SIMD::Pack` (R1).
- `Tape::firstHit()`, `Tape::intersects()` by marching (D3).
- Per CLAUDE.md: unit tests (`TEMPLATE_TEST_CASE`, independent references: known cases against an AABB
  unit cube (D19), closed forms, brute force over all faces), `[gpu]` cases for every ray form and the march; `Tests/InstantiateAll.cpp`;
  one performance probe for a `TriMeshSDF` (D18); Sphinx Concepts + Implem pages; `Examples/RayTracing`
  with its page (D17); the receipt.

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
- [x] Interview: D1–D19 (2026-10-23).
- [ ] #230 (BVH traversal) lands; rebase on it.
- [ ] Core types and free functions lifted from #214/#216.
- [ ] Segment overloads, any-hit `intersects`.
- [ ] March normals.
- [ ] Triangle ray kernel on `SIMD::Pack`.
- [ ] Ray forms: `MeshSDF`/`FlatMeshSDF`, `BVHUnion`.
- [ ] `Tape::firstHit()`/`intersects()`.
- [ ] Tests (host, `[gpu]`), `InstantiateAll`.
- [ ] Performance cases; receipt (host + CUDA).
- [ ] Sphinx pages, Doxygen, example.
- [ ] Delete this file.
