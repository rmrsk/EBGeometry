Examples/ClosestPointTreePacked
-------------------------------

Closest-point search over a point cloud, demonstrating the point-cloud primitive
`PointAoSoA<T, Meta, W>` as a `PackedBVH` leaf (see
[EBGeometry issue #92](https://github.com/rmrsk/EBGeometry/issues/92)), built via
`TreeBVH::packWith()`.

This is the companion to [`Examples/ClosestPointSFCPacked`](../ClosestPointSFCPacked/README.md): it
searches the same 500,000-point cloud with the same 500 queries and compares the same five build
strategies -- **Morton (SFC)**, **Hilbert (SFC)**, **TopDown centroid**, **Midpoint**, and **SAH**
-- against a brute-force scan. The difference is the **construction path**. Where
`ClosestPointSFCPacked` forms the `PointAoSoA<T, size_t, W>` groups up front (space-filling-curve
ordering, then chunking into groups) and builds a `PackedBVH` directly over those pre-formed groups,
this example builds the tree the way the library's own `TriMeshSDF` does:

1. Build a `TreeBVH` over the **individual points**, one primitive per point.
2. Flatten it into a by-value (`ValueStorage`) `PackedBVH` with `TreeBVH::packWith()`, whose
   leaf-conversion callback coalesces each leaf's contiguous run of points into `PointAoSoA` groups
   of `W` in place.

Letting the partitioner see all 500,000 points -- instead of a coarser set of pre-formed groups --
builds a **higher-quality tree**: for the top-down strategies it sharply cuts the leaves visited per
query relative to the corresponding row of `ClosestPointSFCPacked`, so queries are faster, at the
cost of a slower build (the tree is over `W`x as many primitives). Run both examples and compare
their `SAH` rows to see the trade-off.

The five strategies are not all built the same way, which the output makes visible:

* **TopDown centroid, Midpoint, SAH** are top-down (`TreeBVH::topDownSortAndPartition`) and stop
  splitting once a node holds `maxLeafGroups * W` points. Because only the last group of each leaf is
  ever partially filled, their groups stay nearly full and each leaf holds several groups.
* **Morton (SFC)** and **Hilbert (SFC)** are bottom-up (`TreeBVH::bottomUpSortAndPartition<...>()`),
  which build fixed `K`-primitive leaves and ignore `maxLeafGroups`. With `K == W` that is one group
  per leaf, so the tree is much deeper and queries visit many more leaves -- via this path a
  space-filling curve is a poorer match for the `PointAoSoA` grouping than the top-down partitioners.
  Between the two curves, **Hilbert** visits far fewer leaves than **Morton** (its better locality
  produces tighter leaves), so it is the clear winner of the SFC pair here. (`ClosestPointSFCPacked`'s
  SFC rows are a different construction -- `PackedBVH`'s direct SFC constructor with a tunable leaf
  size -- so the two examples' Morton/Hilbert rows are not directly comparable.)

As in `ClosestPointSFCPacked`, the whole search stays in **squared distance** (`getMinimumDistance2()`):
`pruneTraverse()` compares its bound against the *squared* distance to each child box, so no
`sqrt()` appears anywhere on the query path.

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

The binary is `ClosestPointTreePacked.ex`, in this same directory (same as the other two methods
below). Build in single precision, against a library in a different location, or with
`EBGEOMETRY_EXPECT()` runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./ClosestPointTreePacked.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o ClosestPointTreePacked.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

Running
-------

    ./ClosestPointTreePacked.ex

Takes no arguments. It generates a random 500,000-point cloud and 500 query points (fixed seeds, so
results are reproducible on a given machine) and prints a short header followed by one table row per
strategy giving build time, average query time, speedup over brute force, average leaf visits per
query, and the average number of `PointAoSoA` groups per visited leaf. Every query's result is
checked against a brute-force scan with `EBGEOMETRY_EXPECT`, so building with
`-DEBGEOMETRY_ENABLE_ASSERTIONS` aborts on any mismatch; the checks compile out otherwise.

Worth noting when reading the output:

* **Leaf visits are the payoff.** Compared with the matching row of `ClosestPointSFCPacked` (same
  query code, same leaf primitive), the top-down builds here visit fewer leaves per query, because
  the partitioner worked over the full cloud rather than pre-formed groups -- that is where the
  query-time win comes from.
* **The build is slower.** The tree is over `W`x as many primitives (points, not groups), so the
  higher tree quality is paid for at construction time. Whether that trade is worth it depends on how
  many queries amortize each build.
* **Leaf size is tuned via `maxLeafGroups` in `main.cpp`** for the top-down strategies. A leaf-size
  sweep on this workload put the query-time knee around 16-32 groups per leaf; 16 is the default. Try
  8 or 32 to see the effect. (The bottom-up SFC builds are unaffected.)
* `K` (the tree fan-out) and `W` (points per group) are both fixed at 4 -- a good sweet spot
  balancing build and query time; `BVH::DefaultBranchingRatio<T>()` and `PointSoA::DefaultWidth<T>()`
  would instead pick the SIMD-optimal value for the target ISA and precision.
