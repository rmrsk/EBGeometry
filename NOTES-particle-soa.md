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
- **Float precision helps BUILD time; traversal is on par with `double` *at the same SoA width*.**
  Build: `float` ~2× faster than `double` — build is **memory-traffic bound** (see the Midpoint
  finding below: it copies/scans/sorts BV+centroid data, not arithmetic), so halving every value's
  byte size (8B -> 4B) roughly halves it. Traversal: at the hard-coded `W=4`, `float` and `double`
  are **on par** — within a few %, row-dependent, NOT a float regression (float verification passes).
  Precision is a compile-time template parameter (`EBGEOMETRY_PRECISION=float`).
- **The "float traversal is slower" effect is a SoA-WIDTH effect, not precision — and `W=4` fixes
  it.** `DefaultWidth<float>()` on AVX is **8** (vs **4** for `double`), so before pinning `W=4` the
  float examples ran width-8 leaves. For closest-point / nearest-neighbor **min-queries**, the hot
  cost is the per-lane **scalar reduction** (keeping the running best / the `Knn::tryInsert` dedupe +
  insert, plus the reciprocal update in the k-NN example) — NOT the SIMD distance kernel. That scalar
  work scales with `W`: a width-8 leaf does ~2× the per-lane reductions while the query only wants
  the single nearest, most of it wasted. Measured (NearestNeighbor ClusterSAH, 500k, µs/pt):
  `double@4 ~0.885`, `float@4 ~0.867`, **`float@8 ~1.34` (~1.5× slower)**. So hard-coding `W=4`
  *improves* float traversal for these queries — the ISA-"optimal" width was tuned to fill the SIMD
  distance register, but a min-query is bound by the scalar reduction instead. **`W=K=4` is the right
  query-time sweet spot for BOTH precisions.** (Caveat: the effect is query-shape dependent — for the
  cheap single-`min` ClosestPoint reduction, `float@8` was marginally *faster* on query, ~7%, than
  `float@4`; only the heavier k-NN `tryInsert`/reciprocal reduction makes wide groups clearly lose.)

## Recommended all-NN query recipe (what the examples now ship)

The consistently-good combination — every lever that helps-or-is-flat (never hurts) and stays *local*
so it parallelizes over the outer-threaded independent trees:

1. **Seed-from-own-leaf + skip** — seed each point's prune bound from the leaf it already lives in,
   skip that leaf in traversal. Local; **beats reciprocal culling and makes it redundant** (SFCPacked
   SAH, us/pt: reciprocal-only 0.73, seed-only 0.64, seed+reciprocal 0.65). So **drop reciprocal
   culling** — done in both examples now.
2. **Process queries in spatial (Hilbert ≈ leaf) order** — ~35% cache win, free, never hurts.
3. **`tryInsert` reorder + `if constexpr (kNN==1)`** (committed), **W = K = 4**.
4. **Build:** a tight-ish tree — SAH-over-points if build-once/query-many, ClusterSAH if build-bound
   (seed then buys back most of the query gap). **Leaf size ~16 groups for the examples' general
   `Knn` query** — a shallower tree helps, but the optimum shifts with the query-loop cost (below).

Both `Examples/NearestNeighbor{SFC,Tree}Packed` now default to this (seed on, reciprocal removed,
Hilbert order); a "Pre-seed OFF" section shows the win. Measured double, 500k, us/pt (build ms):
SFCPacked SAH 0.65 (97), ClusterSAH 0.71 (41), Midpoint 0.75 (50), Morton 1.54 (55); TreePacked SAH
0.44 (336), Midpoint 0.45 (174), Hilbert 0.85 (303). Caveat: seed is flat on an already-optimal tight
tree — the win concentrates on looser/cheap-build trees, which is where you want it. `maxLeafGroups`
= 16 in the examples, which is query-optimal for their general `Knn` query (32 helps build but
slows query -- see leaf-size finding).

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

- **`Knn::tryInsert` (the hot per-lane scalar reduction of the min-query) was optimized — committed.**
  Two changes in both `Examples/NearestNeighbor*`: (1) test the cheap distance bound
  `count == kNN && a_dist2 >= dist2[kNN-1]` **before** the de-dupe index scan, so the vast majority of
  candidates (farther than the current worst) skip the scan; (2) for `kNN == 1` the de-dupe is
  redundant — a repeat is either the current best (already rejected by the bound) or farther (same) —
  so it is `if constexpr (kNN > 1)`-compiled out. Behavior-preserving for all `kNN` (a candidate only
  reaches the scan if it would be inserted). **~30% faster traversal** (NN ClusterSAH, float, 500k:
  ~0.87 → ~0.61 µs/pt); verified in float + double.
- **Group-min per-group reduction: PROFILED and REJECTED (it is SLOWER).** Idea (kNN==1 only): reduce
  a group's `W` lane distances to the single nearest lane, then one `tryInsert` per group instead of
  `W`. Prototyped and benchmarked — **~30–40% slower** (NN float 500k: ClusterSAH ~0.61 → ~0.87,
  Midpoint ~0.88 → ~1.11 µs/pt) and leaf visits rise slightly (5.7 → 5.9). Two reasons the profiling
  exposed: (a) it does **not** remove the inherent O(`W`) per-lane scan — the optimized `tryInsert`
  fast path is already a single compare, the same cost as the min-compare — while adding
  min-tracking bookkeeping; (b) it **weakens reciprocal culling**, which currently reciprocates
  *every* transiently-accepted lane (seeding many neighbors' bounds); group-min reciprocates only the
  group's single nearest, so bounds tighten slower and more leaves are visited. The per-lane
  reciprocal offers are worth more than the saved inserts. **Closed — do not retry for point clouds.**
- **Seed-from-own-leaf: a local bound-tightener that often beats reciprocal culling — WIRED IN
  (committed) to both `NearestNeighbor*` examples.** For all-NN, every query point already lives in a
  leaf, so descending from the root with a `+inf` bound is wasted. Before traversing, scan the point's
  own leaf, seed its prune bound from it, and **skip that leaf during traversal** (visit once, not
  twice — the skip is essential: seed-without-skip was *slower* on tight trees). A one-time no-prune
  pass (`buildLeafMap`) maps each point → its leaf's group range, reused across all queries. It is
  **local** (no cross-point coupling, no ordering dependence → parallelizes trivially), unlike
  reciprocal culling. Measured (double, 500k, us/pt, reciprocal OFF vs the both-off baseline):
  SFCPacked Morton 4.94→1.58, SAH 0.86→0.64; and it **beats reciprocal culling** on every SFCPacked
  strategy and on the loose TreePacked trees (e.g. SFCPacked SAH: seed 0.64 < reciprocal 0.73). On an
  already-optimal tree at the ~2-leaf floor (TreePacked SAH) it is flat — nothing left to recover, and
  seed+reciprocal stacked is then redundant overhead. Verified float+double. The bottom-up
  "start at the leaf, walk up" variant would hit the same floor and needs parent pointers `PackedBVH`
  lacks, so seeding (which needs nothing new) is the practical form.
- **Leaf size: FEWER groups per leaf is WORSE; but the sweet spot depends on the QUERY-LOOP cost.** In
  a *lean* query (direct min + cached bound) the kNN=1 optimum is ~32–64 groups; for the examples'
  general `Knn` query it is ~16 (reconciliation below). Swept `maxLeafGroups` on the tight SAH-over-points tree with seed+skip (500k,
  double, us/pt): leaf=1→0.53, 8→0.34, **16→0.31 (current), 32→0.29, 64→0.28 (min)**, 128→0.39,
  256→0.41 — a U-shape, and build cost drops monotonically with bigger leaves (leaf=16 ~275 ms vs 64
  ~212 ms). Counterintuitive: shrinking leaves scans FEWER points per query yet is SLOWER. Why:
  **leaf visits stay ~2 regardless of leaf size** (the seeded bound finds the NN in ~2 leaves either
  way), so smaller leaves only make the tree DEEPER — and the cost is the latency-bound tree
  descent (node pointer-chasing), NOT the leaf scan, which is cheap cache-friendly SIMD. So the lever
  that works is **make the tree shallower (bigger leaves)** until the leaf scan itself gets too big
  (~128+ groups). This is the SAME lesson as the bottom-up result: traversal is the cost, leaf/AABB
  work is cheap — cutting individual AABB tests (bottom-up) didn't help, cutting tree DEPTH (bigger
  leaves) does -- but only up to where the per-leaf scan cost bites. **Reconciliation (important):**
  that ~32–64 optimum used the *lean* scratch query. The examples' real query is heavier per lane
  (`Knn::tryInsert` + `worst2()` as the prune bound), so scanning the bigger leaves' extra points
  costs more and the example's query optimum is back at **~16** -- measured: bumping the examples
  16→32 SLOWED query (SFCPacked SAH 0.65→0.71, TreePacked SAH 0.44→0.53) while speeding build. So
  **keep `maxLeafGroups=16` for the examples**; bigger leaves win only with a lean per-point scan.
  Lesson: profile the ACTUAL query loop, not a lean proxy. (Also kNN-specific: larger kNN wants
  bigger leaves regardless.)
- **Bottom-up (leaf-anchored) traversal: PROTOTYPED and REJECTED — cuts AABB tests but no wall-clock
  gain.** Idea (Robert's): every all-NN query point already lives in a leaf, so start AT that leaf and
  walk UP, checking only sibling subtrees at each ancestor — skipping the root→leaf descent and its
  "own-path" AABB tests (which always pass, so are pure overhead). Prototyped via a temporary
  `PackedBVH::getNodes()` accessor + parent / point→leaf-node maps + a custom bottom-up traversal;
  correct (0 mismatches vs top-down, all trees). It DOES cut AABB tests **~18–23%** (ClusterSAH 45→37,
  SAH-over-points 36→28) — exactly the wasteful own-path tests. **But wall-clock is FLAT to slightly
  WORSE across all trees** (tight SAH-over-points 500k: top-down ~0.272 vs bottom-up ~0.283 µs/pt,
  6-run mean; earlier "5% faster" was a noise outlier). Reason: the AABB tests are cheap
  (cache-resident node-bbox ops) and NOT the bottleneck — leaf distance work + memory latency dominate,
  and leaf visits are unchanged (~2.1). Bottom-up also adds overhead (parent walk, per-ancestor sibling
  loop, fragmented descend stacks vs one unified traversal). Net: not worth the machinery; the
  `getNodes()` accessor was reverted. **Top-down seed+skip is effectively optimal** — the descent is
  "wasteful" in test count but cheap in time. Closed.
- **ClusterSAH build↔query knee swept (`maxClusterSize`), lean seed+skip query, SFC-packed groups,
  double 500k:** mcs=2 → 117 ms/0.53, 4 → 66/0.47, **8 (default) → 50/0.435, 16 → 23/0.424**, 32 →
  19/0.469, 64 → 17/0.61. **Knee at mcs≈16: build ~23 ms (2.5× FASTER than nanoflann's 57 ms), query
  0.42 (~1.6× nanoflann)** — a strong operating point; mcs=8 is PAST the knee (2× build for the same
  query). **BUT the leaf-size trap strikes a third time:** this free lunch holds only for the LEAN
  query. In the examples' general-`Knn` query, mcs 8→16 REGRESSED query (0.71→0.81 us/pt, grp/leaf
  10.7→19.7) while halving build — bigger clusters = bigger leaves = heavier per-lane `tryInsert`
  scan. So examples keep mcs=8 (query-optimal for their query). **The recurring lean-vs-general
  divergence is the signal: specializing the query to lean kNN=1 (direct min + cached bound) unlocks
  the cheaper build too** (bigger clusters/leaves become free) — build 23 ms AND query 0.42
  simultaneously. That is the compounding case for the lean-query specialization.
- **Tree-build profiling (closing the ~6× build gap vs nanoflann's 57 ms).** SAH-over-points build
  (~390 ms, double 500k) decomposes: make_shared wrap+AABBs 29 ms (7%), TreeBVH ctor 15 ms (4%),
  **SAH `topDownSortAndPartition` 330 ms (79%)**, packWith 42 ms (10%). So the *tight-tree* build is
  dominated by the SAH partitioner -- NOT the shared_ptr plumbing (that was the *Midpoint* story,
  where the split is trivial). Binned SAH over 500k points is inherently ~5-6× nanoflann's
  median-split kd-tree build. Levers profiled/identified:
  - **(a) longest-axis SAH -- IMPLEMENTED (committed): `BinnedSAHPartitioner<T,P,BV,K,true>`.** Bins
    only the longest centroid-bbox axis, not all 3. MEASURED **~21% off the SAH build** (395->312 ms)
    with **ZERO query cost** (0.31 vs 0.32 us/pt, same leaf visits). Added as an optional final
    template flag `LongestAxisOnly` (default false, so 3-axis stays the default -- may matter more for
    mesh BVHs); threaded through `SAH2WaySplit`/`SAHKWaySplit`. Test + docs added, all 269 tests pass.
  - **(b) index-based build** (drop the per-primitive `make_shared`, partition an index permutation):
    recovers the ~11% wrap+ctor phases.
  - **(c) radix-sort the Morton codes** (`SFCImplem.hpp:280` is a comparison `std::sort` on integer
    codes) -- speeds the fast SFC-packed builds and ClusterSAH's clustering.
  - The pragmatic **cheap-tight-build already exists: ClusterSAH** (~40-125 ms build, 0.52 us/pt query
    = SAH over *clusters* not points). The real gap-closer (SAH-quality tree at LBVH build speed) is
    **LBVH + treelet reoptimization** -- a real project, untried. For per-step rebuild (moving
    particles) use ClusterSAH/Midpoint; tight SAH-over-points is for build-once/query-many.
- **PointCloudBVH: LANDED (committed).** `PointCloudBVH<T, Meta, K, W> : public PackedBVH<T,
  PointAoSoA<T,size_t,W>, K, ValueStorage<...>>` -- a subclass that (a) builds via the index-based
  copy-free Midpoint build (below) directly from positions + a parallel user-metadata array, using a
  new **protected `PackedBVH(nodes&&, prims&&)` adopt-arrays ctor**, and (b) exposes turnkey queries
  hiding pruneTraverse: `closestPoint`/`closestPoints` (external), `nearestNeighbor`/`nearestNeighbors`
  (self, seed-from-own-leaf), `allNearestNeighbors(k)` (batch, Hilbert-ordered). Leaves carry the
  cloud INDEX; user Meta stored alongside (metadata()/position() accessors). Build ~87 ms/500k (near
  nanoflann's 57). Tests (TestPointCloudBVH, both precisions vs brute force), InstantiateAll, doxygen,
  and an ImplemBVH.rst mention all in. **Follow-ups:** rewrite the ClosestPoint*/NearestNeighbor*
  examples to use it (they collapse to a few lines); dedicated Sphinx example page.
- **Query "2x above benchmark" -- ROOT-CAUSED and FIXED.** The single-nearest hot path
  (closestPoint/nearestNeighbor/allNearestNeighbors(1)) used the base `pruneTraverse`, which is tuned
  for expensive-leaf SDF/mesh queries: at every interior node it does a **near-first ordered descent**
  -- a per-node `std::sort` of the K children, driven by a SIMD child-AABB kernel that reads a
  separate `m_childAabbSoA` side array (~4 MB, thrashes L2). For cheap SoA point leaves the ordering
  costs more than the nodes it prunes (it cuts 35.8 -> 10.9 nodes/pt, but net slower). **Fix:** the
  k==1 path now uses a lean **unordered scalar DFS** over the packed nodes (k>1 stays on pruneTraverse,
  where the k-best insert dominates and the tighter bound helps). Traversal-only, 500k self-NN,
  double: pruneTraverse 0.35 -> lean **0.28** us/pt (~20% win; even beats the 0.315 prototype -- the
  class tree is slightly tighter, 2.16 vs 2.22 leaf/pt). The apparent extra "2x vs prototype" was a
  **harness artifact**: a repeat-call benchmark charged `allNearestNeighbors`'s once-per-call
  `SFC::order<Hilbert>` (500k sort) + 8 MB `vector<Hit>` alloc on every rep, which the prototype
  hoisted out of its timer. Measured attribution: Hit-width 3% (not a factor), query()-call boundary
  ~0%, the rest was that hoisted sort+alloc.
- **Index-based build: PROTOTYPED, closes the build gap (76 ms vs 275-390 ms).** Mirrored nanoflann:
  partition a single `uint32` index array **in place** by longest-axis Midpoint, pack each leaf's
  points into `PointGroup`s **inline** during the build -- no `make_shared`, no per-node sublist
  copies, no separate `packWith`, no `TreeBVH`. Point->own-leaf map falls out of the build for free
  (so seeding needs no extra pass). Measured (double 500k): **build 76 ms** (vs current
  Midpoint-over-points ~275, SAH-over-points ~390; nanoflann ~57), **query 0.32 us/pt** (2.22 leaf/pt,
  as tight as SAH; nanoflann ~0.26), **0/400 brute-force mismatches**. So an index-based Midpoint build
  puts EBGeometry **within ~1.3x of nanoflann on BOTH build (76 vs 57) and query (0.32 vs 0.26)** --
  from a general BVH, down from ~6.8x build / ~1.3x query for SAH-over-points. **Confirms definitively:
  the build gap was machinery, not algorithm.** Prototype: /tmp scratch (FlatNode array + inline
  packing). Next: integrate as a new direct `PackedBVH` point-cloud build path (write straight into
  `m_linearNodes`/`m_primitives`); a pool/bump allocator + trimming the leaf-pack could chase the
  remaining ~19 ms to nanoflann.
- **How nanoflann builds its kd-tree (studied its source for build hints).** (1) Partitions a single
  `std::vector<uint32_t> vAcc_` of INDICES **in place** -- no per-primitive alloc, no `shared_ptr`, no
  per-node sublist copies; points read via an accessor. (2) **Pool-allocated nodes** (bump allocator).
  (3) Split (`middleSplit_`) = **longest-axis MIDPOINT**: max-spread dim via one unrolled min/max
  scan, split at the bbox midpoint clamped to [min,max] and nudged toward count/2 for balance, one
  Dutch-flag `planeSplit` pass -- **no SAH, no sort**. So nanoflann's kd-tree ≈ our
  `MidpointPartitioner`, over individual points, index-based. Hints:
  - **(1) SAH is unnecessary for the tight tree (MEASURED).** Over individual points on a uniform
    cloud, Midpoint / BVCentroid / SAH all give **identical query** (~0.34 us/pt, ~2.1 leaf/pt) -- the
    tightness comes from point-granularity partitioning, not from SAH. So the ~330 ms SAH is wasted
    for point clouds; **Midpoint (or longest-axis SAH) over points is as tight**, and cheaper.
  - **(2) The remaining gap is build MACHINERY, not the algorithm.** Even Midpoint-over-points is
    ~200-275 ms vs nanoflann's ~57 ms for the *same* algorithm -- the difference is EBGeometry's
    per-primitive `make_shared` (~29 ms) + per-node sublist copies through the pointer `TreeBVH` +
    `packWith` (~42 ms, the SoA-group price). nanoflann's index-in-place + pool-alloc has none of it.
  - **The real improvement: an index-based, copy-free, pool-allocated top-down build (nanoflann/LBVH
    style) with a cheap Midpoint split.** Attacks the plumbing that dominates once SAH is dropped.
    Real Source project; the design to mirror is nanoflann `divideTree`/`middleSplit_`/`planeSplit`
    over an index permutation. (`packWith`'s ~42 ms is inherent to the SIMD SoA-group query rep.)
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
