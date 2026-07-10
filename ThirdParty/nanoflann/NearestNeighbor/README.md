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

Each strategy is queried in **two processing orders** -- natural index order and Hilbert-sorted --
to expose the query-ordering (cache-locality) lever; the result is order-independent, only the time
changes. It cross-checks that every strategy agrees with nanoflann on each point's nearest
**distance** (indices may differ on exact ties).

### Reading the result

On a typical AVX2 machine (500k points, single core) nanoflann is roughly **~2x faster than
EBGeometry on both build and query** for pure point NN -- the expected outcome of a *specialized
kd-tree* versus a *general BVH* (the same `PackedBVH` EBGeometry uses for mesh SDFs and CSG). The
**SAH-over-points** tree is EBGeometry's closest: query within ~13% of nanoflann (natural order), but
at ~6x the build cost; SAH/ClusterSAH over pre-made groups stay ~2.4x on query. So the kd-tree wins
the combined build+query, as expected.

**Query ordering** is a real ~30-50% wall-clock lever: processing points in Hilbert order instead of
natural index order gives every strategy a large cache-locality speedup (see the `SFC gain` column).
Note it is *not* an EBGeometry advantage -- nanoflann's tighter nodes exploit locality even more (it
gains ~50% vs EBGeometry's ~35%), so SFC ordering slightly *widens* the same-order gap rather than
closing it. It is still worth doing in a real particle code (queries ~1.5x faster), and there the
particles are often already spatially coherent.

Precision matters mostly on build: `float` roughly halves EBGeometry's build (it is memory-traffic
bound) while nanoflann's index build is precision-insensitive; query is ~unchanged.

### Building and running

nanoflann is fetched automatically (single header, `curl`) on first build.

    make                      # double precision
    make PRECISION=float      # single precision
    ./NearestNeighbor.ex

Override the path to the EBGeometry root (the directory containing `EBGeometry.hpp`) with
`EBGEOMETRY_HOME=/path/to/EBGeometry` if not building in place. `make realclean` also removes the
fetched `nanoflann.hpp`.
