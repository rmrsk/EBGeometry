ThirdParty/nanoflann/NearestNeighbor
------------------------------------

A head-to-head benchmark of **EBGeometry's `PackedBVH` against the
[nanoflann](https://github.com/jlblancoc/nanoflann) kd-tree** on the same all-nearest-neighbor
problem: for every point in a 500,000-point random cloud, find its single nearest *other* point
(`kNN = 1`, self excluded). Both structures are built and queried **single-threaded from the
identical cloud**, so the numbers are directly comparable.

> **Note.** This is a throwaway comparison harness, not a supported part of EBGeometry, and it is not
> exercised by CI. It exists to sanity-check EBGeometry's point-query performance against a
> purpose-built reference.

### Why nanoflann, and why single-threaded

nanoflann is a compact, well-regarded **kd-tree specialized for nearest-neighbor** over point clouds
-- exactly this problem -- so it is the fair apples-to-apples reference for the query side (a kd-tree
partitions space with no bounding-volume overlap and has very cheap nodes, so it is expected to beat
a *general* BVH on pure point queries).

Single-threaded is the honest comparison here because it matches the intended deployment: in an
embedded-boundary / particle code the parallelism is **outer** -- each core owns its own particle
cloud and builds and queries its own independent tree concurrently -- so per-core, single-tree
performance is the number that matters, and there is no inner loop to thread.

### What it measures

* **EBGeometry**, three build strategies over the same cloud, all queried with per-point
  `pruneTraverse()` (independent queries, no reciprocal culling, to match nanoflann):
  * **SAH (points)** -- a `TreeBVH` built over the *individual* points (binned SAH), then
    `packWith()` into `PointAoSoA<T, size_t, 4>` groups. Partitions at point granularity, so the
    tightest tree -- at the highest build cost.
  * **SAH (groups)** and **ClusterSAH** -- built over pre-made Morton-packed groups (top-down binned
    SAH, and density-adaptive clustering respectively). Cheaper builds, but they can only partition
    at group granularity, so looser trees.
* **nanoflann**: `KDTreeSingleIndexAdaptor` (L2, 3-D, `leaf_max_size = 16`, single-threaded build);
  query is per-point `knnSearch(k = 2)` taking the second result (the first is the point itself at
  distance 0).

Every EBGeometry query runs in **Hilbert (spatial) order** -- the in-order processing that is part
of the recommended strategy -- and is timed both as a plain **top-down** traversal (bound from
`+inf`) and with our **seed-from-own-leaf + skip** optimization (each point's bound seeded from the
leaf it already lives in, that leaf skipped in traversal; `buildLeafMap` builds the point->leaf map
once per tree). nanoflann is timed **vanilla** in natural order (typical usage) and, for an
equal-footing tree comparison, in the same Hilbert order. Results are cross-checked identical.

### Reading the result

With our full optimization stack (seed-from-own-leaf + skip, Hilbert-order queries, `W=K=4`, lean
`kNN=1` query loop) on the tight **SAH-over-points** tree, EBGeometry queries at **~0.33-0.35 us/pt**
(500k, double) -- versus vanilla nanoflann at **~0.58 us/pt** in natural order or **~0.27** if its
queries are also spatially pre-sorted. So the comparison depends on what "vanilla" means:

* **vs vanilla nanoflann (natural-order queries):** EBGeometry is **~1.7x faster** on query (0.35 vs
  0.58). Visit-in-order is one of *our* optimizations; a nanoflann user who does not reorder queries
  pays the natural-order price.
* **vs nanoflann on the same spatial ordering:** nanoflann is **~1.3x faster** on query (0.27 vs
  0.35) -- the specialized kd-tree still wins apples-to-apples, but our optimizations narrowed the
  query gap from the ~2x an unoptimized traversal shows to ~1.3x.
* **Build:** EBGeometry's tight tree is **~6x** nanoflann's build (SAH-over-points ~360 ms vs ~57 ms);
  the cheap-build **ClusterSAH** is ~2x nanoflann's build and queries at ~0.5 us/pt.

seed-from-own-leaf itself is worth ~20-35% (e.g. SAH-groups 0.70 -> 0.47, ClusterSAH 0.77 -> 0.52).
The honest bottom line: a *general* BVH carrying our optimizations either beats or is within ~1.3x of
a *specialized* kd-tree on query (depending on the ordering assumption), at a higher build cost.

### Building and running

nanoflann is fetched automatically (single header, `curl`) on first build.

    make                      # double precision
    make PRECISION=float      # single precision
    ./NearestNeighbor.ex

Override the path to the EBGeometry root (the directory containing `EBGeometry.hpp`) with
`EBGEOMETRY_HOME=/path/to/EBGeometry` if not building in place. `make realclean` also removes the
fetched `nanoflann.hpp`.
