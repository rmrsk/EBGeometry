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
*every* point in the cloud, its `kNN = 1` nearest *other* points. Each point walks the tree with
`pruneTraverse()`, keeping a kNN-slot sorted set of nearest neighbors whose current kNN-th distance is the
pruning bound. A spread-out sample of points is checked against a brute-force scan.

Its companion, [`Examples/NearestNeighborTreePacked`](../NearestNeighborTreePacked/README.md), solves
the identical problem over groups formed the other way (a `TreeBVH` over individual points, then
`packWith()`), and shows a different reciprocal-culling profile.

Reciprocal distance culling
---------------------------

The k-NN graph is symmetric: if B is near A, then A is near B, at the *same* distance. The example
exploits this. Points are processed in Hilbert order (so a point's neighbors are handled close
together in time), and whenever point A's traversal *accepts* B into A's kNN nearest, it immediately
offers A back to B's neighbor set. That tightens B's pruning bound **before B is ever traversed**, so
when B's turn comes it prunes more aggressively and visits fewer leaves. Reciprocating only *accepted*
neighbors (not every far candidate the traversal happens to touch) keeps the bookkeeping cheap.

This is always correct: a neighbor set only ever holds *real* points at *real* distances, so its kNN-th
distance is always a valid upper bound on the true kNN-th-nearest distance -- pruning by it never drops a
true neighbor. Reciprocal offers to points already finished are harmless no-ops.

The output's **Reciprocal culling OFF** rows re-run the same BVHs with the reciprocal step disabled.
Comparing them against the main rows shows the payoff: **culling reduces leaf visits in proportion to
how loose the baseline traversal is.** The SFC-direct builds (Morton/Hilbert) form comparatively loose
trees over the pre-made groups and visit many leaves whose points aren't actually near the query, so
pre-seeding each point's true neighbors prunes those leaves sharply; the tighter SAH build has less to
gain. (Take the raw squared distances as the SIMD kernel gives them: `PointAoSoA::getDistances2()`
returns all `W` lane distances at once, and `getMetaData()` names each lane's cloud index.)

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
**Reciprocal culling OFF** rows follows. A spread-out sample of 500 points has its kNN neighbors
checked against a brute-force scan with `EBGEOMETRY_EXPECT`, so building with
`-DEBGEOMETRY_ENABLE_ASSERTIONS` aborts on any mismatch; the checks compile out otherwise.

Worth noting when reading the output:

* **The speedup baseline is estimated from the verification sample.** Full all-pairs brute force is
  O(N²); the per-point brute-force cost is measured on the 500-point sample and extrapolated, so the
  "Brute force" microseconds-per-point (and every speedup) is representative, not a full N² run.
* **Leaf visits drive query time**, and the reciprocal culling reduces them -- most for the loose
  Morton/Hilbert builds, least for SAH. See the ON vs OFF rows.
* **`kNN` is hard-coded to 1** (`kNN` in `main.cpp`); `maxLeafGroups` tunes the leaf size. `K` (tree
  fan-out) and `W` (points per group) are fixed at 4 -- a good sweet spot.
