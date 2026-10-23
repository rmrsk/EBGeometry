<!--
SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>

SPDX-License-Identifier: GPL-3.0-or-later
-->

# BVH traversal: development document

> **To be deleted before merge.** This is the live working document for the `bvh_traversal` branch and
> its pull request: decisions with their reasons, the proposed interface, open points, and progress.
> What survives the work goes into the Sphinx docs (`ImplemBVH.rst` and its Concepts counterpart), the
> Doxygen comments and `Examples/BVHTraversal`; this file does not.

## Goal

One general, user-facing BVH traversal, `PackedBVH::traverse()`, in which the user chooses:

- **the query**: what is tracked through the tree. A point, a ray or segment, or a type of their own
  (their own child test). One query per walk; many queries are the user's own loop, or one per thread
  in their own `GPU::transform` functor.
- **the visit order**: nearest first, farthest first, as stored, or their own.
- **the state and the evaluators**: what is remembered between leaves, what happens at a leaf, and the
  bound a node must be within to be visited.

It replaces today's `traverse()` (four callbacks, `std::function`-compatible) and `pruneTraverse()`
(point queries, nearest first), so there is one way to walk a `PackedBVH`. It must land before the ray
queries (#228), which become ordinary users of it (a ray query, a leaf evaluator that tests triangles).

## Why

- The ray work (#228) needs a fast walk for rays; #214 used today's `traverse()`, which tests one child
  box at a time and recomputes keys through a callback.
- `pruneTraverse()` has the fast parts (all K child boxes tested in one SIMD pass from the parent's SoA
  row, nearest first, one stack entry per level) but only for points.
- Writing a third walk for rays would duplicate the stack logic; getting the structure wrong now would
  make every later query type a cleanup.

## Decisions (interview, 2026-10-23)

| # | Decision | Reason |
|---|---|---|
| T1 | **Its own PR, before #228.** #228 is rebased on it. | The ray API builds on the walk; the walk is judged and measured alone. |
| T2 | **Query types, extensible.** The library ships query kinds with a SIMD K-children kernel each; a user writes their own query type with the same small interface. | One walk for every query; new kinds (frustum, swept sphere, box) need no change to the walk. |
| T3 | **Replace both `traverse()` and `pruneTraverse()`.** No deprecation path (dev is in crunch mode); every internal user migrated in this PR. | One way to walk a BVH. |
| T4 | **No launcher.** The user calls `traverse()` per query in their own loop or functor (one query per thread on a device). | The library does not own the user's data layout. |
| T5 | ~~A packet provides its lane count and a child test~~ **dropped with packets (T16).** | |
| T6 | **One state object per walk, the user's type**; the leaf callback updates it, the bound callback reads it. | As `pruneTraverse()` today. |
| T7 | **Visit order is a policy**: `NearestFirst` (default), `FarthestFirst`, `AsStored` (no sort: any-hit, all-hits), or a user comparator on the keys. | Closest-hit and nearest-surface searches want nearest first; any-hit and collect-all do not need a sort. |
| T8 | **One stack entry per level; keys recomputed on return**, as `pruneTraverse()` does now: parent and child order; the next child's key is recomputed from the parent's row, bit for bit. | Small stack in device local memory. |
| T9 | **Query types in this PR: `PointQuery` and `RayQuery`** (a segment is a ray whose initial bound is its length). | What the existing users and the ray work need. |
| T10 | ~~One walk; a single query is a packet of one lane~~ **dropped with packets (T16).** | |
| T11 | **No `std::function` callbacks.** Template parameters only (functors, lambdas); the `PackedBVH` aliases are removed. | They cannot run on a device and block inlining. |
| T12 | **`pruneTraverse()` is removed**; its callers use `traverse()` with a point query (nearest first is the default). | One way to walk. |
| T13 | **#202**: `mortonOrder()` and the ordered `GPU::transform` are kept (they order queries so neighbours walk similar paths) and land with or after this PR; `pruneTraverseBatch` is dropped; #202 is closed as superseded, its branch kept. | The batch launcher wraps the removed `pruneTraverse()`, and T4 rules out launchers. |
| T14 | **Example: `Examples/BVHTraversal`**: the general traverse with the user's own state and leaf evaluator, one query per thread, a point query (nearest primitive) and a ray query (first box hit), on a plain `PackedBVH`. | CLAUDE.md asks for an example for a new capability; the ray example (#228) then shows `firstHit()` on a `TriMeshSDF`. |
| T15 | ~~Packets of at most 32 lanes~~ **dropped with packets (T16).** | |
| T16 | **No packets.** One query per walk. The SIMD stays where it is today: one query against the K child boxes of a node, and (in the ray work) one ray against the W triangles of a leaf. The door for packets is left open (see *Room for packets later*). | Packets do not add SIMD (they move it from children and triangles to rays); they only amortise node fetches for coherent queries, which our main uses (particles, EB paths, GPU threads) are not. They would add lane masks to the walk, the stack and every callback. Decided 2026-10-23. |

Vocabulary: a **packet** (several queries walked together by one thread) is not in this PR (T16). A
**batch** (#202's word: many queries, one per thread) is the user's own loop or functor.

## Interface (proposed, for review before implementation)

Declarations only, in `namespace EBGeometry::BVH`, all `EBGEOMETRY_HOST_DEVICE` and `noexcept`.

### The walk

```cpp
template <class T, class P, size_t K>
class PackedBVH
{
public:
  // Walks the tree with one query, depth first.
  //   a_query : a query type (see "Query types"); gives each child box a key.
  //   a_state : the caller's search state. Only a_leaf changes it.
  //   a_leaf  : (State&, size_t offset, size_t count) -> void. Called at each leaf reached; scans
  //             primitives [offset, offset+count) of getPrimitives().
  //   a_bound : (const State&) -> T. The current bound, re-read at every node: a child is entered only if
  //             its key is <= the bound. A bound below every possible key (e.g. -infinity) ends the walk
  //             (any-hit after the first crossing).
  //   a_order : NearestFirst (default), FarthestFirst, AsStored, or a comparator on child keys.
  template <class Query, class State, class LeafEvaluator, class BoundFn, class Order = NearestFirst>
  void traverse(const Query& a_query, State& a_state, LeafEvaluator&& a_leaf, BoundFn&& a_bound,
                Order a_order = Order()) const noexcept;

  // The SoA boxes of an interior node's K children, read-only, for query types (and users) that test
  // them: m_lo[axis][child], m_hi[axis][child]. Public now; private today.
  struct ChildBoxes;
};

struct NearestFirst {};
struct FarthestFirst {};
struct AsStored {};
```

### Query types

A query type is any type with this interface (the walk calls nothing else):

```cpp
struct MyQuery
{
  using Scalar = T;

  // The keys of all K children of one node at once (the SIMD kernel): +infinity for a child the query
  // cannot reach. Called when a node is expanded; the walk compares the keys with the bound.
  template <size_t K>
  Array<T, K> keys(const ChildBoxes<T, K>& a_row) const;

  // One child's key again, when the walk returns to its level: bit for bit what keys() gave.
  template <size_t K>
  T key(const ChildBoxes<T, K>& a_row, size_t a_k) const;

  // The root's key (the root has no parent row).
  T key(const AABBT<T>& a_box) const;
};
```

Shipped in this PR (T9):

| Type | Key for a child box | Notes |
|---|---|---|
| `PointQuery<T>` | squared distance from the point to the box | today's `pruneTraverse()` |
| `RayQuery<T>` | distance along the ray where it enters the box (slab test over [tMin, ∞)), +inf if it misses | origin, 1/direction, tMin; a segment is a ray whose initial bound is its length |

The ray type holds no `RayT`/`HitT`: those belong to the ray queries (#228), which build on it.

### Use

```cpp
// Nearest primitive to a point (what pruneTraverse() did):
struct Nearest { T best2 = inf; uint32_t id = none; };

Nearest state;
bvh.traverse(PointQuery<T>{p}, state,
             [&](Nearest& s, size_t offset, size_t count) { /* distance to each primitive */ },
             [](const Nearest& s) { return s.best2; });                       // NearestFirst

// First box hit along a ray:
struct FirstHit { T t = tMax; uint32_t id = none; };

FirstHit hit;
bvh.traverse(RayQuery<T>{origin, direction, tMin}, hit,
             [&](FirstHit& s, size_t offset, size_t count) { /* test each primitive, keep the nearest */ },
             [](const FirstHit& s) { return s.t; });

// Any hit: the bound drops to -infinity once there is a crossing; AsStored skips the sort.
bvh.traverse(RayQuery<T>{origin, direction, tMin}, anyState, leaf, bound, AsStored{});
```

### Room for packets later (T16)

Packets are not built, but the interface does not rule them out, so they can be added without breaking
anyone if a measurement ever calls for them:

1. **Callbacks keep their single-query signatures** (`leaf(State&, offset, count)`, `bound(const State&)
   -> T`). The walk reads a query's lane count through a trait that defaults to 1, so a query type does
   not declare it. A packet query would get the masked forms (`leaf(State&, LaneMask, offset, count)`,
   `bound(...) -> Array<T, N>`) as additional forms chosen by `if constexpr`; nothing existing changes.
2. **A query type gives keys; the walk decides who enters** (key <= bound). A packet query would
   implement a separate, wider interface (per-lane keys and masks), recognised through the same trait.
3. **The stack and its entries stay private**: a packet's lane mask per level is internal.
4. **No public name a packet would need** (`RayPacket`, `PointPacket`, `LaneMask`) is used now.

The cost now: the lane-count trait (default 1), one `static_assert` that it is 1, and a test that a user
query type without the trait works.

## Points to confirm

- **`TreeBVH`** (host-only, pointer tree) keeps its own `traverse()` and `std::function` aliases; only
  `PackedBVH`'s change. (`TreeBVH` is a build structure; nothing in the hot path uses it.)
- **Child boxes become public** (read-only), since user query types must read them.
- **Names**: `traverse`, `PointQuery`/`RayQuery`, `NearestFirst`/`FarthestFirst`/`AsStored`,
  `ChildBoxes`.

## Scope

- The walk (T6–T8), the order policies (T7), the two query types (T9), the room for packets (T16).
- Migrate every user: `MeshSDF`, `TriMeshSDF` (`MeshDistanceFunctionsImplem.hpp`), `BVHUnion`,
  `PointCloudBVH`, the Tape's union clauses, the tests.
- Remove `pruneTraverse()`, the old `traverse()` and the `PackedBVH` `std::function` aliases (T3, T11,
  T12).
- `mortonOrder()` and the ordered `GPU::transform` from #202 (T13), if they fit here.
- Per CLAUDE.md: unit tests against brute force (point: nearest box; ray: first box entered; every order
  policy; any-hit; a user query type), `[gpu]` cases, `Tests/InstantiateAll.cpp`; the receipt (host +
  CUDA), where every existing distance query must stay unchanged; new probe cases for ray walks;
  `ImplemBVH.rst` and the Concepts page; `Examples/BVHTraversal` with its Sphinx page (T14).

## Sequence

1. Interface review (this document).
2. The walk with `PointQuery`, migrating every `pruneTraverse()` caller; receipt: no change.
3. The ray type, the order policies; removal of the old API.
4. Tests, probe cases, docs, example; final receipt.
5. #228 (ray queries) rebased on it.

## Progress

- [x] Interview (T1–T16; packets dropped), branch `bvh_traversal` off `dev` (8e03593), draft PR, this document.
- [ ] Interface reviewed.
- [ ] The walk + `PointQuery`; `pruneTraverse()` callers migrated; receipt unchanged.
- [ ] Ray type, order policies.
- [ ] Old API removed; `traverse()` callers migrated.
- [ ] `mortonOrder()` / ordered `GPU::transform` (T13).
- [ ] Tests (host, `[gpu]`), `InstantiateAll`.
- [ ] Probe cases; receipt (host + CUDA).
- [ ] Sphinx, Doxygen, `Examples/BVHTraversal`.
- [ ] Delete this file.
