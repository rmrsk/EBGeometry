Examples/ClosestPointPacked
---------------------------

Closest-point search over a point cloud, demonstrating the point-cloud primitive
`PointAoSoA<T, Meta, W>` as a `PackedBVH` leaf (see
[EBGeometry issue #92](https://github.com/rmrsk/EBGeometry/issues/92)), built via
`TreeBVH::packWith()`.

50,000 random points in the unit cube are searched. Each carries its cloud index as metadata, and
`W` (`PointSoA::DefaultWidth<T>()`) is the SIMD-optimal group width for the target ISA and
precision. 500 query points are resolved via `pruneTraverse()` and checked against a brute-force
scan.

The point of this example is the **construction path**. Rather than forming the
`PointAoSoA<T, size_t, W>` groups up front and building a BVH over them -- what its companion
[`Examples/ClosestPointSFC`](../ClosestPointSFC/README.md) does -- this example builds the tree the
way the library's own `TriMeshSDF` does:

1. Build a `TreeBVH` over the **individual points**, one primitive per point, partitioned with the
   surface-area heuristic (`BinnedSAHPartitioner`) down to `maxLeafGroups * W` points per leaf.
2. Flatten it into a by-value (`ValueStorage`) `PackedBVH` with `TreeBVH::packWith()`, whose
   leaf-conversion callback coalesces each leaf's contiguous run of points into `PointAoSoA` groups
   of `W` in place.

Letting SAH partition all 50,000 points -- instead of a coarser set of ~12,500 pre-formed groups --
builds a **higher-quality tree**: it visits noticeably fewer leaves per query, so queries are faster,
at the cost of a slower build (the tree is over 4x as many primitives). Because a leaf holds up to
`maxLeafGroups * W` points and only the last group per leaf is ever partially filled, the groups
also stay nearly full (average occupancy close to `W`, printed in the output). Run this example and
`ClosestPointSFC` and compare their `SAH` rows to see the build-time-vs-query-time trade-off.

As in `ClosestPointSFC`, the whole search stays in **squared distance** (`getDistance2()`):
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

The binary is `ClosestPointPacked.ex`, in this same directory (same as the other two methods below).
Build in single precision, against a library in a different location, or with
`EBGEOMETRY_EXPECT()` runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./ClosestPointPacked.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o ClosestPointPacked.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

Running
-------

    ./ClosestPointPacked.ex

Takes no arguments. It generates a random 50,000-point cloud and 500 query points (fixed seeds, so
results are reproducible on a given machine), prints a short header and the group-occupancy achieved
by `packWith()`, then a table row giving build time, average query time, speedup over brute force,
average leaf visits per query, and the average number of `PointAoSoA` groups per visited leaf. Every
query's result is checked against a brute-force scan with `EBGEOMETRY_EXPECT`, so building with
`-DEBGEOMETRY_ENABLE_ASSERTIONS` aborts on any mismatch; the checks compile out otherwise.

Worth noting when reading the output:

* **Leaf visits are the payoff.** Compared with the `SAH` row of `ClosestPointSFC` (same query code,
  same leaf primitive), this build visits fewer leaves per query, because SAH partitioned the full
  cloud rather than pre-formed groups -- that is where the query-time win comes from.
* **The build is slower.** The tree is over `W`x as many primitives (points, not groups), so the
  higher tree quality is paid for at construction time. Whether that trade is worth it depends on how
  many queries amortize each build.
* **Leaf size is tuned via `maxLeafGroups` in `main.cpp`.** A leaf-size sweep on this workload put
  the query-time knee around 16-32 groups per leaf; 16 is the default. Try 8 or 32 to see the effect.
* `K` (the tree fan-out) and `W` (points per group) both derive from the ISA via
  `BVH::DefaultBranchingRatio<T>()` and `PointSoA::DefaultWidth<T>()`, so the SIMD box test and leaf
  distance test each fill one register.
