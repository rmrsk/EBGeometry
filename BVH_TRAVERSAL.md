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

- **the query**: what is tracked through the tree. A point, a ray or segment, or a type of their own.
  One query per thread, or a **packet** of up to 32 queries per thread that share one walk. The user
  organises the data either way; the library ships query types for points and rays (single and packet)
  and the user can write their own (their own layout, their own child test).
- **the visit order**: nearest first, farthest first, as stored, or their own.
- **the state and the evaluators**: what is remembered between leaves, what happens at a leaf, and the
  bound a node must be within to be visited.

It replaces today's `traverse()` (four callbacks, `std::function`-compatible) and `pruneTraverse()`
(point queries, nearest first), so there is one way to walk a `PackedBVH`. It must land before the ray
queries (#228), which become ordinary users of it (a ray query, a ray packet, a leaf evaluator that
tests triangles).

## Why

- The ray work (#228) needs a fast walk for rays, single and packed; #214 used today's `traverse()`,
  which tests one child box at a time and recomputes keys through a callback.
- `pruneTraverse()` has the fast parts (all K child boxes tested in one SIMD pass from the parent's SoA
  row, nearest first, one stack entry per level) but only for points.
- Writing a third walk for rays, and a fourth for packets, would duplicate the stack logic; getting the
  structure wrong now would make every later query type a cleanup.

## Decisions (interview, 2026-10-23)

| # | Decision | Reason |
|---|---|---|
| T1 | **Its own PR, before #228.** #228 is rebased on it. | The ray API builds on the walk; the walk is judged and measured alone. |
| T2 | **Query types, extensible.** The library ships query kinds with a SIMD K-children kernel each; a user writes their own query type with the same small interface. | One walk for every query; new kinds (frustum, swept sphere, box) need no change to the walk. |
| T3 | **Replace both `traverse()` and `pruneTraverse()`.** No deprecation path (dev is in crunch mode); every internal user migrated in this PR. | One way to walk a BVH. |
| T4 | **No launcher.** The user organises the data as singles or packets and calls `traverse()` per thread in their own functor; the library provides `RayPacket`/`PointPacket`, the user may write their own packet. | The library does not own the user's data layout. |
| T5 | **A packet provides its lane count and a child test**: given a node's K child boxes and the active lanes, it returns per child a key and the mask of lanes that enter it. | Vector work over the packet's own layout, no gathers. |
| T6 | **One state object per walk, the user's type.** For a packet it can hold per-lane arrays; the bound callback returns per-lane bounds, the leaf callback gets the active-lane mask. | Same shape for singles and packets. |
| T7 | **Visit order is a policy**: `NearestFirst` (default), `FarthestFirst`, `AsStored` (no sort: any-hit, all-hits), or a user comparator on the keys. A packet orders children by their smallest key over its active lanes. | Closest-hit and nearest-surface searches want nearest first; any-hit and collect-all do not need a sort. |
| T8 | **One stack entry per level; keys recomputed on return**, as `pruneTraverse()` does now: parent, child order and (packets) the level's lane mask; the next child's key and lane mask are recomputed from the parent's row, bit for bit. | Small stack in device local memory. |
| T9 | **Query types in this PR: point and ray/segment, single and packet.** `RayPacket` (and the ray query) live here, not in #228. | What the existing users and the ray work need. |
| T10 | **One walk; a single query is a packet of one lane**, with masks and lane loops compiled away (`if constexpr`) at N = 1. | One implementation; the receipt checks N = 1 costs nothing against today's `pruneTraverse()`. |
| T11 | **No `std::function` callbacks.** Template parameters only (functors, lambdas); the `PackedBVH` aliases are removed. | They cannot run on a device and block inlining. |
| T12 | **`pruneTraverse()` is removed**; its callers use `traverse()` with a point query (nearest first is the default). | One way to walk. |
| T13 | **#202**: `mortonOrder()` and the ordered `GPU::transform` are kept (they group coherent queries into packets) and land with or after this PR; `pruneTraverseBatch` is dropped; #202 is closed as superseded, its branch kept. | The batch launcher wraps the removed `pruneTraverse()`, and T4 rules out launchers. |
| T14 | **Example: `Examples/BVHTraversal`**: the general traverse with the user's own state and leaf evaluator, one query and a packet per thread, a point query (nearest primitive) and a ray query (first box hit), on a plain `PackedBVH`. | CLAUDE.md asks for an example for a new capability; the ray example (#228) then shows `firstHit()` on a `TriMeshSDF`. |
| T15 | **Packets have at most 32 lanes**: a lane mask is one `uint32_t`. 4, 8 and 16 are tested and measured. | Wider packets diverge quickly on real data. |

Vocabulary: a **packet** is several queries walked together by one thread. A **batch** (#202's word) is
many queries, one per thread; this PR does not add batches.

## Interface (proposed, for review before implementation)

Declarations only, in `namespace EBGeometry::BVH`, all `EBGEOMETRY_HOST_DEVICE` and `noexcept`.

### The walk

```cpp
template <class T, class P, size_t K>
class PackedBVH
{
public:
  // Walks the tree with a query (one lane or a packet), depth first.
  //   a_query   : a query type (see "Query types"), single or packet; it decides which children each
  //               lane enters and gives each child a key.
  //   a_state   : the caller's search state, one object for the whole query (per-lane arrays inside
  //               for a packet). Only a_leaf changes it.
  //   a_leaf    : (State&, LaneMask active, size_t offset, size_t count) -> void. Called at each leaf
  //               some lane reaches, with the lanes that reach it; scans primitives [offset, offset+count)
  //               of getPrimitives().
  //   a_bound   : (const State&) -> Bounds<T, Lanes>. Each lane's current bound, re-read at every node:
  //               a lane enters a child only if its key for that child is <= its bound. A bound below
  //               every possible key (e.g. -infinity) ends that lane (any-hit after its first crossing).
  //   a_order   : NearestFirst (default), FarthestFirst, AsStored, or a comparator on child keys.
  template <class Query, class State, class LeafEvaluator, class BoundFn, class Order = NearestFirst>
  void traverse(const Query& a_query, State& a_state, LeafEvaluator&& a_leaf, BoundFn&& a_bound,
                Order a_order = Order()) const noexcept;

  // The SoA boxes of an interior node's K children, read-only, for query types (and users) that test
  // them: m_lo[axis][child], m_hi[axis][child]. Public now; private today.
  struct ChildBoxes;
};

using LaneMask = uint32_t;                 // bit i: lane i

template <class T, uint32_t Lanes>
using Bounds = Array<T, Lanes>;            // one bound per lane (a single query: one element)

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
  static constexpr uint32_t Lanes = N; // 1 for a single query, up to 32 for a packet

  // All K children of one node at once (the SIMD kernel). For each child k: enters[k] = the lanes of
  // `active` whose key for child k is <= bound[lane]; key[k] = the smallest such key (what the order
  // sorts by). Called when a node is expanded.
  template <size_t K>
  void children(const ChildBoxes<T, K>& a_row, LaneMask a_active, const Bounds<T, Lanes>& a_bound,
                Array<T, K>& a_key, Array<LaneMask, K>& a_enters) const;

  // One child again, when the walk returns to its level: bit for bit the same key and lanes as
  // children() would give with the same mask and bounds.
  template <size_t K>
  T child(const ChildBoxes<T, K>& a_row, size_t a_k, LaneMask a_active, const Bounds<T, Lanes>& a_bound,
          LaneMask& a_enters) const;

  // The root's box (it has no parent row).
  T root(const AABBT<T>& a_box, LaneMask a_active, const Bounds<T, Lanes>& a_bound, LaneMask& a_enters) const;
};
```

Shipped in this PR (T9):

| Type | Lanes | Key for a child box | Notes |
|---|---|---|---|
| `PointQuery<T>` | 1 | squared distance from the point to the box | today's `pruneTraverse()` |
| `PointPacket<T, N>` | N | the same, per lane | SoA: x[N], y[N], z[N] |
| `RayQuery<T>` | 1 | distance along the ray where it enters the box (slab test), +inf if it misses | origin, 1/direction, tMin; a segment is a ray whose initial bound is its length |
| `RayPacket<T, N>` | N | the same, per lane | SoA origins, directions, tMin, tMax; `setRay()`, `setSegment()` |

The ray types hold no `RayT`/`HitT`: those belong to the ray queries (#228), which build on these.

### Use

```cpp
// Nearest primitive to a point (what pruneTraverse() did):
struct Nearest { T best2 = inf; uint32_t id = none; };

bvh.traverse(PointQuery<T>{p}, state,
             [&](Nearest& s, LaneMask, size_t offset, size_t count) { /* distance to each primitive */ },
             [](const Nearest& s) { return Bounds<T, 1>{s.best2}; });            // NearestFirst

// First box hit along each of 16 rays, one packet per thread:
struct FirstHits { Array<T, 16> t; Array<uint32_t, 16> id; };

RayPacket<T, 16> rays; /* setRay() per lane */
bvh.traverse(rays, hits,
             [&](FirstHits& s, LaneMask active, size_t offset, size_t count) { /* test lanes in active */ },
             [](const FirstHits& s) { return s.t; });                              // per-lane bound

// Any hit: the bound drops to -infinity for a lane once it has a crossing; AsStored skips sorting.
bvh.traverse(rays, anyState, leaf, bound, AsStored{});
```

## Points to confirm

- **The leaf callback always takes the lane mask**, also for a single query (where it is always 1), so
  one signature serves both. Alternatively a single query's leaf callback could omit it.
- **`TreeBVH`** (host-only, pointer tree) keeps its own `traverse()` and `std::function` aliases; only
  `PackedBVH`'s change. (`TreeBVH` is a build structure; nothing in the hot path uses it.)
- **Child boxes become public** (read-only), since user query types must read them.
- **Bounds type**: `Array<T, Lanes>` for every query, or a plain `T` for a single query.
- **Names**: `traverse`, `PointQuery`/`PointPacket`/`RayQuery`/`RayPacket`, `NearestFirst`/
  `FarthestFirst`/`AsStored`, `LaneMask`, `Bounds`.

## Scope

- The walk (T5–T8, T10), the order policies (T7), the four query types (T9).
- Migrate every user: `MeshSDF`, `TriMeshSDF` (`MeshDistanceFunctionsImplem.hpp`), `BVHUnion`,
  `PointCloudBVH`, the Tape's union clauses, the tests.
- Remove `pruneTraverse()`, the old `traverse()` and the `PackedBVH` `std::function` aliases (T3, T11,
  T12).
- `mortonOrder()` and the ordered `GPU::transform` from #202 (T13), if they fit here.
- Per CLAUDE.md: unit tests against brute force (point: nearest box; ray: first box entered; single and
  packets at N = 1, 4, 8, 16, 32; every order policy; any-hit; a packet whose lanes diverge), `[gpu]`
  cases, `Tests/InstantiateAll.cpp`; the receipt (host + CUDA), where every existing distance query
  must stay unchanged; new probe cases for ray and packet walks; `ImplemBVH.rst` and the Concepts page;
  `Examples/BVHTraversal` with its Sphinx page (T14).

## Sequence

1. Interface review (this document).
2. The walk with `PointQuery`, migrating every `pruneTraverse()` caller; receipt: no change.
3. Packets, the ray types, the order policies; removal of the old API.
4. Tests, probe cases, docs, example; final receipt.
5. #228 (ray queries) rebased on it.

## Progress

- [x] Interview (T1–T15), branch `bvh_traversal` off `dev` (8e03593), draft PR, this document.
- [ ] Interface reviewed.
- [ ] The walk + `PointQuery`; `pruneTraverse()` callers migrated; receipt unchanged.
- [ ] Packets, ray types, order policies.
- [ ] Old API removed; `traverse()` callers migrated.
- [ ] `mortonOrder()` / ordered `GPU::transform` (T13).
- [ ] Tests (host, `[gpu]`), `InstantiateAll`.
- [ ] Probe cases; receipt (host + CUDA).
- [ ] Sphinx, Doxygen, `Examples/BVHTraversal`.
- [ ] Delete this file.
