Examples/NearestNeighborTreePacked
----------------------------------

The **kNN nearest neighbors of every point** in a random cloud (the classic k-NN graph), using the
point-cloud primitive `PointAoSoA<T, Meta, W>` as a `PackedBVH` leaf (see
[EBGeometry issue #92](https://github.com/rmrsk/EBGeometry/issues/92)), built via
`TreeBVH::packWith()`.

This is the k-nearest-neighbor counterpart to
[`Examples/ClosestPointTreePacked`](../ClosestPointTreePacked/README.md), and the companion to
[`Examples/NearestNeighborSFCPacked`](../NearestNeighborSFCPacked/README.md): it solves the same
all-kNN-nearest-neighbors problem over the same 500,000-point cloud and the same five build strategies,
but forms the `PointAoSoA<T, size_t, W>` groups the other way -- a `TreeBVH` is built over the
*individual* points and the groups are materialised from its leaves via `TreeBVH::packWith()`
(**Morton**/**Hilbert** bottom-up, **TopDown**/**Midpoint**/**SAH** top-down). Each point walks the
tree with `pruneTraverse()`, keeping a kNN-slot sorted set of nearest neighbors whose kNN-th distance is
the pruning bound; a spread-out sample is checked against a brute-force scan.

Reciprocal distance culling
---------------------------

The k-NN graph is symmetric: if B is near A, then A is near B, at the *same* distance. Points are
processed in Hilbert order, and whenever point A's traversal *accepts* B into A's kNN nearest, it
immediately offers A back to B's neighbor set -- tightening B's pruning bound **before B is ever
traversed**, so B's later traversal prunes harder. Only *accepted* neighbors are reciprocated (not
every far candidate), keeping it cheap. This is always correct: a neighbor set holds only real points
at real distances, so its kNN-th distance is a valid upper bound on the true kNN-th-nearest distance, and
pruning by it never drops a true neighbor.

The output's **Reciprocal culling OFF** rows re-run the same BVHs with the reciprocal step disabled.
The comparison shows the key insight: **culling reduces leaf visits in proportion to how loose the
baseline traversal is.**

* **Morton (SFC)** and **Hilbert (SFC)** are bottom-up over individual points, so their leaves hold
  only `K` points -- a point's own leaf does *not* fill its kNN-neighbor set, the search stays loose,
  and culling cuts leaf visits sharply (often halving them or more).
* **SAH** (and the other top-down builds) partition the full cloud into a very tight tree that
  already visits close to the minimum number of leaves, so there is almost nothing left to cull --
  culling on/off are nearly indistinguishable.

(This differs from `NearestNeighborSFCPacked`, whose looser trees-over-groups let culling help *every*
strategy, SAH included -- so which regime you are in depends on the construction, not just on `kNN`.)

Distances come straight from the SIMD kernel: `PointAoSoA::getDistances2()` returns all `W` lane
squared distances to the query at once, and `getMetaData()` names each lane's cloud index.

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

The binary is `NearestNeighborTreePacked.ex`, in this same directory (same as the other two methods
below). Build in single precision, against a library in a different location, or with
`EBGEOMETRY_EXPECT()` runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./NearestNeighborTreePacked.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o NearestNeighborTreePacked.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

Running
-------

    ./NearestNeighborTreePacked.ex

Takes no arguments. It generates a random 500,000-point cloud (fixed seed, so results are
reproducible on a given machine) and prints a short header followed by one table row per strategy
giving build time, average time to find one point's kNN neighbors, speedup over brute force, average
leaf visits per point, and the average number of `PointAoSoA` groups per visited leaf. A section of
**Reciprocal culling OFF** rows follows. A spread-out sample of 500 points has its kNN neighbors
checked against a brute-force scan with `EBGEOMETRY_EXPECT`, so building with
`-DEBGEOMETRY_ENABLE_ASSERTIONS` aborts on any mismatch; the checks compile out otherwise.

Worth noting when reading the output:

* **The speedup baseline is estimated from the verification sample.** Full all-pairs brute force is
  O(N²); the per-point brute-force cost is measured on the 500-point sample and extrapolated.
* **The build is slower than `NearestNeighborSFCPacked`'s** for the top-down strategies -- the tree
  is over `W`x as many primitives (points, not groups) -- but that same full-cloud partitioning is
  what makes the top-down trees tight (few leaf visits per query).
* **`kNN` is hard-coded to 1** (`kNN` in `main.cpp`); `maxLeafGroups` tunes the top-down leaf size (the
  bottom-up SFC builds ignore it). `K` (tree fan-out) and `W` (points per group) are fixed at 4 -- a good sweet spot.
