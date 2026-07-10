Examples/NearestNeighborSFCPacked
---------------------------------

The **kNN nearest neighbors of every point** in a random cloud (the classic k-NN graph), using the
point-cloud primitive `PointAoSoA<T, Meta, W>` as a `PackedBVH` leaf (see
[EBGeometry issue #92](https://github.com/rmrsk/EBGeometry/issues/92)).

This is the k-nearest-neighbor counterpart to
[`Examples/ClosestPointSFCPacked`](../ClosestPointSFCPacked/README.md): same organization -- 500,000
points SFC-sorted and chunked into `PointAoSoA<T, size_t, W>` groups, then built into a `PackedBVH`
six ways (**Morton (SFC)**, **Hilbert (SFC)**, **TopDown centroid**, **Midpoint**, **SAH**, and
**ClusterSAH** -- density-adaptive clustering of the groups, then SAH over the clusters, which reaches
SAH-level tree quality at a much lower build cost) via the direct constructors -- but a different
query. Instead of one closest point per query, it finds, for
*every* point in the cloud, its `kNN = 1` nearest *other* point. Each point walks the tree with
`pruneTraverse()`, keeping a kNN-slot sorted set whose current kNN-th distance is the pruning bound.
A spread-out sample of points is checked against a brute-force scan.

Its companion, [`Examples/NearestNeighborTreePacked`](../NearestNeighborTreePacked/README.md), solves
the identical problem over groups formed the other way (a `TreeBVH` over individual points, then
`packWith()`), and shows how the strategy behaves on a tighter tree.

Seed-from-own-leaf (the query strategy)
---------------------------------------

Since this is *all*-nearest-neighbor, every query point already *lives in a leaf* of the BVH -- so
descending from the root with a `+inf` bound is wasted work. Instead, before traversing, scan that
own leaf, **seed** the point's pruning bound from it, and **skip that leaf during traversal** (visit
it once, not twice). The descent then prunes with an already-tight bound. A single no-prune pass
(`buildLeafMap`) maps each point to its leaf's group range up front, reused across all queries. Points
are processed in **Hilbert order** so consecutive queries touch nearby leaves and keep the tree nodes
(and the seeded own-leaf data) hot in cache.

This is always correct: the seed only ever puts *real* points at *real* distances into the neighbor
set, so its kNN-th distance is a valid upper bound on the true kNN-th-nearest distance -- pruning by
it never drops a true neighbor.

It is also **local** -- each point seeds only from its own leaf, with no cross-point coupling and no
dependence on processing order, so it parallelizes trivially (matching the intended deployment, where
each core owns its own cloud and tree). This replaces the *reciprocal distance culling* an earlier
version used (a global scheme that offered each accepted neighbor back to tighten the other point's
bound): for all-nearest-neighbor, seed-from-own-leaf is simpler and consistently at least as fast.
The output's **Pre-seed OFF** rows re-run a few strategies with plain top-down traversal (bound
starting at `+inf`) to show the gain -- large on loose trees (the descent otherwise wanders through
many leaves before finding a tight bound), small on an already-tight tree near the ~2-leaf floor.

(Distances come straight from the SIMD kernel: `PointAoSoA::getDistances2()` returns all `W` lane
squared distances at once, and `getMetaData()` names each lane's cloud index.)

Building
--------

This example is standalone and can be built in three ways. Each needs the path
to the EBGeometry root -- the directory that contains `EBGeometry.hpp` -- which
is two levels up from this folder (`../..`) when building in place. See
[Direct compilation](https://rmrsk.github.io/EBGeometry/BuildingDirectCompile.html),
[Building with GNU Make](https://rmrsk.github.io/EBGeometry/BuildingGNUMake.html), and
[Building with CMake](https://rmrsk.github.io/EBGeometry/BuildingCMake.html) in the user
documentation for more detail on each approach.

**CMake**

    cmake -S . -B build
    cmake --build build

The binary is `NearestNeighborSFCPacked.ex`, in this same directory (same as the other two methods
below). Build in single precision, against a library in a different location, or with
`EBGEOMETRY_EXPECT()` runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./NearestNeighborSFCPacked.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o NearestNeighborSFCPacked.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

Running
-------

    ./NearestNeighborSFCPacked.ex

Takes no arguments. It generates a random 500,000-point cloud (fixed seed, so results are
reproducible on a given machine) and prints a short header followed by one table row per strategy
giving build time, average time to find one point's kNN neighbors, speedup over brute force, average
leaf visits per point, and the average number of `PointAoSoA` groups per visited leaf. A section of
**Pre-seed OFF** rows follows. A spread-out sample of 500 points has its kNN neighbors
checked against a brute-force scan with `EBGEOMETRY_EXPECT`, so building with
`-DEBGEOMETRY_ENABLE_ASSERTIONS` aborts on any mismatch; the checks compile out otherwise.

Worth noting when reading the output:

* **The speedup baseline is estimated from the verification sample.** Full all-pairs brute force is
  O(N²); the per-point brute-force cost is measured on the 500-point sample and extrapolated, so the
  "Brute force" microseconds-per-point (and every speedup) is representative, not a full N² run.
* **Leaf visits drive query time**, and pre-seeding the bound from the own leaf reduces them -- most
  for the loose Morton/Hilbert builds, least for the already-tight SAH. See the main vs Pre-seed OFF rows.
* **`kNN` is hard-coded to 1** (`kNN` in `main.cpp`); `maxLeafGroups` tunes the leaf size. `K` (tree
  fan-out) and `W` (points per group) are fixed at 4 -- a good sweet spot.
