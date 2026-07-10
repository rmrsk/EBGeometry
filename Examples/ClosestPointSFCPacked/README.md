Examples/ClosestPointSFCPacked
------------------------------

Closest-point search over a point cloud, demonstrating the point-cloud primitive
`PointAoSoA<T, Meta, W>` as a `PackedBVH` leaf (see
[EBGeometry issue #92](https://github.com/rmrsk/EBGeometry/issues/92)).

500,000 random points in the unit cube are Morton-sorted and packed into `PointAoSoA<T, size_t, W>`
groups of up to `W` points each (`W` is `PointSoA::DefaultWidth<T>()`, the SIMD-optimal width for
the target ISA and precision), with each point carrying its cloud index as metadata. The *same*
groups are then built into a `PackedBVH` five ways -- **Morton (SFC)**, **Hilbert (SFC)**,
**TopDown centroid**, **Midpoint**, and **SAH** -- via the `PackedBVH` direct constructors, so their
closest-point query performance can be compared head to head. 500 query points are resolved against
each via `pruneTraverse()` and checked against a brute-force scan.

This is the **space-filling-curve** construction path: the groups are formed up front from an
SFC ordering of the cloud, and the `PackedBVH` is then built directly over those pre-formed groups
(no `TreeBVH`). Its companion,
[`Examples/ClosestPointTreePacked`](../ClosestPointTreePacked/README.md), forms the same kind of
groups the other way around -- building a `TreeBVH` over the *individual* points first and
materializing the `PointAoSoA` groups from each leaf via `TreeBVH::packWith()`. Running both and
comparing their `SAH` rows shows the trade-off between the two.

**Morton vs Hilbert.** The two space-filling-curve rows are the interesting comparison here: both
order the groups along a curve, but the Hilbert curve has no long-range jumps (consecutive codes are
always spatially adjacent), so it produces more compact groups and tree nodes. On this workload
`Hilbert (SFC)` visits noticeably fewer leaves per query than `Morton (SFC)` for a near-identical
build cost -- a direct illustration of why Hilbert is often preferred for spatial ordering.

One detail is worth calling out, since it is the crux of using these primitives efficiently: the
whole search stays in **squared distance** (`getDistance2()`). `pruneTraverse()` compares its bound
against the *squared* distance to each child box, so no `sqrt()` appears anywhere on the query path;
a linear bound would be a unit mismatch that prunes far too loosely.

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

The binary is `ClosestPointSFCPacked.ex`, in this same directory (same as the other two methods
below). Build in single precision, against a library in a different location, or with
`EBGEOMETRY_EXPECT()` runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./ClosestPointSFCPacked.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o ClosestPointSFCPacked.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

Running
-------

    ./ClosestPointSFCPacked.ex

Takes no arguments. It generates a random 500,000-point cloud and 500 query points (fixed seeds, so
results are reproducible on a given machine) and prints a short header followed by one table row per
strategy giving build time, average query time, speedup over brute force, average leaf visits per
query, and the average number of `PointAoSoA` groups per visited leaf. Every query's result is
checked against a brute-force scan with `EBGEOMETRY_EXPECT`, so building with
`-DEBGEOMETRY_ENABLE_ASSERTIONS` aborts on any mismatch; the checks compile out otherwise.

Worth noting when reading the output:

* **Leaf visits drive query time.** Every strategy queries the same points with the same
  `pruneTraverse()` code and differs only in the tree it builds. A tighter tree (SAH, Hilbert)
  visits fewer leaves and queries faster, generally at the cost of a slower build -- the query-time
  flip side of `Examples/BuildBVH`, which compares the same strategies' build times.
* **Leaf size is tuned via `maxLeafGroups` in `main.cpp`.** A leaf-size sweep on this workload put
  the query-time knee around 16-32 groups per leaf; 16 is the default. Try 8 or 32 to see the effect.
* **Speedup grows with the point count.** At 500,000 points brute force is only hundreds of
  microseconds, so the speedup is already large; the BVH's per-query cost grows only logarithmically
  while brute force grows linearly, so the gap widens further for larger clouds (raise `numPoints`).
* `K` (the tree fan-out) and `W` (points per group) both derive from the ISA via
  `BVH::DefaultBranchingRatio<T>()` and `PointSoA::DefaultWidth<T>()`, so the SIMD box test and leaf
  distance test each fill one register.
