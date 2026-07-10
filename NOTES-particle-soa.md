# Session notes — PointCloudSoA / BVH build methods (branch `particle_soa_pr3`, issue #92)

Working notes carried in the branch so the work can be resumed on another machine with no local
context. **Delete this file before the PR merges** (it is a hand-off note, not project doc).

Last updated: 2026-07-10.

## Converged conclusion (this is the decision — do not re-open without reason)

- **Keep the ClusterSAH build method.** It stays in the library and in the examples.
- **`K = W = 4` (BVH branching factor and SoA width) is a good sweet spot** — it balances build and
  query time well across strategies, and the point-cloud examples now **hard-code both to 4** (with
  the ISA-adaptive `DefaultBranchingRatio<T>()` / `DefaultWidth<T>()` left as a trailing comment)
  rather than deriving them from the compiled ISA. On AVX/`double` the default already *was* 4; the
  fix pins it for `float`/other ISAs too, so example timings are comparable across machines.
- **Build and traversal times are now within acceptable ranges** — this line of investigation is
  done.
- **SAH and Midpoint are the fastest methods.** Between the two *construction paths*:
  - **slight edge to SFC point-packing (the direct SFC-build path) if BUILD time matters;**
  - **top-down construction if TRAVERSAL / query time matters** — top-down trees are tighter, so
    fewer leaf visits per query. This is illustrated by the two `Examples/NearestNeighbor*`
    programs (compare their Build-ms vs Leaf-visits columns).
- **Float precision helps BUILD time only, not traversal.** Measured: `float` build is ~2× faster
  than `double` (i.e. `double` build takes about twice as long), while **traversal is about the same
  in both — actually a slight edge to `double`.** Precision is a compile-time template parameter
  (`EBGEOMETRY_PRECISION=float`). Why this shape: build is **memory-traffic bound** (see the Midpoint
  finding below — it is dominated by copying/scanning/sorting BV+centroid data, not arithmetic), so
  halving every value's byte size (`double` 8B -> `float` 4B) roughly halves it. Traversal is
  **latency / branch bound** (pointer-chasing nodes, comparing a few distances), so byte size barely
  matters; and `float`'s slightly looser tree (less precise split planes / BVs at build) gives
  `double` a marginal traversal edge. Net rule: **use `float` when build time dominates; it buys
  nothing (costs a hair) at query time.**

## What ClusterSAH is

Density-adaptive clustering of primitives (recursive centroid-bbox midpoint split on the longest
axis — an adaptive octree, stopping at `<= maxClusterSize` primitives per cluster), then a
self-contained 32-bin SAH over the *cluster* boxes. Reaches ~SAH tree quality at a fraction of SAH's
build cost. Direct `PackedBVH` ctor, disambiguated by a `ClusterSpec` argument:

```cpp
const Packed bvh(primsAndBVs, EBGeometry::BVH::ClusterSpec{maxClusterSize});
```

An earlier fixed-Cartesian-grid version was rejected — it collapses on clustered/surface inputs
(hundreds of primitives per query); the adaptive octree is robust (~10–19 per query). See the
failed spatial "middle-out" and HLBVH experiments (noted separately) for what was already ruled out.

## Supporting findings (so they are not re-derived)

- **Midpoint's dominant build cost is plumbing, not the split.** `Midpoint2WaySplit` /
  `MidpointKWaySplit` (`Source/EBGeometry_BVH.hpp`) are a centroid-bbox scan + one `std::partition`
  per level — trivially cheap. Time goes to: (1) up-front `std::make_shared<P>` wrapping of every
  primitive (N heap allocations); (2) the per-node stack-local `TreeBVH probe(a_prims)` copy in the
  direct top-down ctor (`Source/EBGeometry_BVHImplem.hpp`, the `build` lambda ~L698–727) — element
  copies are **atomic `shared_ptr` refcount** increments; (3) recomputing each node's bounding
  volume at every node. To make Midpoint build faster you must attack the plumbing (partition an
  index permutation into one contiguous primitive array, incremental node BVs, drop the probe) —
  NOT the split. Judged not worth it for now ("the performance improvement isn't that great").
- **Direct-ctor "deterioration" investigation is CLOSED.** Deterioration A — a by-value `TreeBVH`
  list ctor pessimizing the probe into copy-into-parameter + move-into-members — was FOUND and FIXED
  by splitting the ctor into const-ref (copy) + rvalue (move) overloads (committed). Residual
  deterioration B is item (2) above (probe still copies the sublist), a ~2.5% gap vs TreeBVH+pack
  for cheap-split strategies; left unfixed on purpose.
- **Midpoint (and all top-down partitioners) work for 3D shapes, not only point clouds.** They
  partition by **BV centroid** (`a_list[i].second.getCentroid()`), never touching the primitive —
  generic over `P`. This is **object partitioning, not spatial**: primitives are assigned wholly to
  a child by centroid, never clipped, so child BVs may **overlap** (valid for a BVH; only
  tightness/query cost suffers). No straddling-primitive "fitting" problem — that only arose in the
  failed *spatial* middle-out prototype. Point clouds are the degenerate zero-volume-BV case.

## Concrete branch state (committed + pushed to `origin/particle_soa_pr3`)

- ClusterSAH direct ctor `PackedBVH(list, BVH::ClusterSpec{maxClusterSize})` in the library; present
  as a row in `Examples/BuildBVH` and `Examples/NearestNeighborSFCPacked`.
- TreeBVH list ctor split into copy/move overloads (deterioration-A fix).
- `Examples/NearestNeighbor{SFC,Tree}Packed`: neighbor-count parameter renamed `k` -> `kNN` and set
  to **1** (was 3), across code, doc comments, and the two READMEs (READMEs now written against the
  `kNN` parameter rather than the literal). Type name `Knn` / `knnPass` and the term "k-NN" kept.
- All four point-cloud examples (`ClosestPoint{SFC,Tree}Packed`, `NearestNeighbor{SFC,Tree}Packed`):
  `W` and `K` hard-coded to **4** (Default*() kept as trailing comment); `numPoints` is **500,000**.
  `BuildBVH` already fixed `K = 4`; `NestedBVH` intentionally left on the default (its outer-union K
  matches its inner mesh BVHs' default, so pinning only the outer would desync them).

## Follow-ups still open (not blocking; do only if asked)

- ClusterSAH lacks: a dedicated `Tests/TestBVH.cpp` case (both precisions), `Tests/InstantiateAll.cpp`
  coverage, and an `Docs/Sphinx/source/Implem*BVH*.rst` mention — add before the PR merges (per
  CLAUDE.md's "new public class" checklist).
- Plumbing rework to fully reclaim direct-build's edge (index-permutation build) is deferred.

## Process reminders (from CLAUDE.md)

Never create branches/PRs/worktrees without explicit approval; committing to the current branch is
fine. When asked to open a PR: `gh pr create --draft`, use `.github/pull_request_template.md`, leave
the reviewer checklist unchecked. The `.rst` docs audit begins once the PR leaves draft. Do not
clang-format `.txt`/CMake files.
