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
tree with `pruneTraverse()`, keeping a kNN-slot sorted set whose kNN-th distance is the pruning bound;
a spread-out sample is checked against a brute-force scan.

Seed-from-own-leaf (the query strategy)
---------------------------------------

Since this is *all*-nearest-neighbor, every query point already *lives in a leaf* of the BVH -- so
descending from the root with a `+inf` bound is wasted work. Instead, before traversing, scan that
own leaf, **seed** the point's pruning bound from it, and **skip that leaf during traversal** (visit
it once, not twice). The descent then prunes with an already-tight bound. A single no-prune pass
(`buildLeafMap`) maps each point to its leaf's group range up front, reused across all queries. Points
are processed in **Hilbert order** so consecutive queries touch nearby leaves and keep the tree nodes
(and the seeded own-leaf data) hot in cache.

It is **local** -- each point seeds only from its own leaf, with no cross-point coupling and no
dependence on processing order, so it parallelizes trivially (matching the intended deployment, where
each core owns its own cloud and tree). It replaces the *reciprocal distance culling* an earlier
version used (a global scheme offering each accepted neighbor back to tighten the other point's bound);
for all-nearest-neighbor, seed-from-own-leaf is simpler and consistently at least as fast.

The output's **Pre-seed OFF** rows re-run a few strategies with plain top-down traversal (bound from
`+inf`), so the gain is visible. Its size depends on how tight the tree is:

* **Morton (SFC)** and **Hilbert (SFC)** are bottom-up over individual points, so their leaves hold
  only `K` points and the descent from `+inf` stays loose -- pre-seeding cuts leaf visits sharply.
* **SAH** (and the other top-down builds) partition the full cloud into a very tight tree already near
  the ~2-leaf minimum, so there is almost nothing left to recover -- pre-seed on/off are nearly
  indistinguishable. (`NearestNeighborSFCPacked`, whose trees-over-groups are looser, benefits on
  every strategy including SAH -- so which regime you are in depends on the construction.)

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
**Pre-seed OFF** rows follows. A spread-out sample of 500 points has its kNN neighbors
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
