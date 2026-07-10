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

* **EBGeometry**: SFC (Morton) pack of the cloud into `PointAoSoA<T, size_t, 4>` groups, then a
  `PackedBVH` built with the density-adaptive **ClusterSAH** direct constructor; query is per-point
  `pruneTraverse()` (independent queries, no reciprocal culling, to match nanoflann's independent
  queries). The reported build time covers the whole raw-points-to-queryable pipeline, and is broken
  down into the SFC-pack and the ClusterSAH halves.
* **nanoflann**: `KDTreeSingleIndexAdaptor` (L2, 3-D, `leaf_max_size = 16`, single-threaded build);
  query is per-point `knnSearch(k = 2)` taking the second result (the first is the point itself at
  distance 0).

It cross-checks that both agree on every point's nearest **distance** (indices may differ on exact
ties) and prints the build- and query-time ratios.

### Reading the result

On a typical AVX2 machine (500k points, single core) nanoflann is roughly **~2x faster than
EBGeometry on both build and query** -- the expected outcome of a specialized kd-tree versus a
general BVH. Being within ~2x of a dedicated kd-tree, while `PackedBVH` is the *same* structure
EBGeometry uses for mesh SDFs and CSG, is a reasonable place to be. Two levers narrow the query gap
that this harness does not exercise: a tighter top-down (SAH) tree trades higher build cost for fewer
leaf visits per query, and the **reciprocal distance culling** in
[`Examples/NearestNeighborSFCPacked`](../../../Examples/NearestNeighborSFCPacked/README.md) cuts
query time further when computing *every* point's neighbor as a batch (nanoflann has no equivalent).
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
