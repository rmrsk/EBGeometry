Examples/ClosestPoint
---------------------

This folder demonstrates nearest-neighbor search over a point cloud using the point-cloud
primitives `PointSoAT<T, W>` and `PointAoSoA<T, Meta, W>` (see
[EBGeometry issue #92](https://github.com/rmrsk/EBGeometry/issues/92)).

50,000 random points are generated in the unit cube, each carrying its own index into that
50,000-point list as metadata. The points are sorted along a Morton space-filling curve and
chunked into `PointAoSoA<T, int, W>` groups of up to `W` points each (`W` is
`PointSoA::DefaultWidth<T>()`, the SIMD-optimal width for the compiling machine's ISA and
precision), so each group's points are spatially close together rather than an arbitrary slice of
the input order. These groups are the BVH's leaf primitives.

The *same* groups are then built into a `PackedBVH` **four** different ways -- one per BVH build
strategy EBGeometry offers over a flat primitive list -- so their nearest-neighbor query
performance can be compared head to head:

* **Morton (SFC)** -- the direct space-filling-curve constructor (`PackedBVH(prims, leafSize,
  SFC::Morton{})`);
* **TopDown centroid** -- the direct top-down constructor with the default bounding-volume-centroid
  partitioner;
* **Midpoint** -- the direct top-down constructor with the sort-less midpoint-split partitioner;
* **SAH** -- the direct top-down constructor with the binned Surface-Area-Heuristic partitioner
  (the same partitioner `TriMeshSDF` uses by default).

500 random query points are then resolved against each `PackedBVH` via `pruneTraverse()`, tracking
a running **squared** distance as the pruning bound (no `sqrt()` anywhere in the hot path -- taken
only once, at the very end, to report the final distance) and reading each winning group's metadata
(`getMetaData()`) only after the search already knows which group won. Every result is checked
against a brute-force linear scan, and each strategy's build time, average per-query time, speedup
over brute force, and average leaf-node visits per query are reported side by side.

The pruning bound is **squared distance specifically** for a reason worth calling out, since it is
the crux of using these primitives efficiently: `pruneTraverse()` compares the bound against the
*squared* distance from the query to each child's bounding box. Feeding it a linear distance
instead is a unit mismatch, and in the unit cube -- where every distance is `< 1`, so squaring
*shrinks* it -- the bound comes out far too loose to prune, ballooning the leaf-visit count by
roughly an order of magnitude (while still returning correct answers). Getting a fast query out of
`PointAoSoA` therefore hinges on keeping the whole search in squared distance via `getDistance2()`,
which is exactly what this example does.

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

The binary is `ClosestPoint.ex`, in this same directory (same as the other two methods below).
Build in single precision, against a library in a different location, or with
`EBGEOMETRY_EXPECT()` runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./ClosestPoint.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o ClosestPoint.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

Running
-------

    ./ClosestPoint.ex

This example takes no arguments. It generates a random 50,000-point cloud and 500 random query
points (fixed seeds, so results are reproducible run to run on the same machine), verifies every
`PackedBVH` nearest-neighbor result (all four strategies) against a brute-force scan, and prints:

* Whether all four strategies agree with brute force on every query's nearest distance (the example
  aborts if any does not) and how often the nearest *point's identity* (its metadata index) also
  matched -- expected to be all queries, barring the vanishingly unlikely case of a query point
  sitting exactly equidistant between two different cloud points.
* The average nearest-neighbor distance found, and the target leaf size in use.
* A table with one row per strategy (plus brute force), giving build time, average per-query time,
  speedup over brute force, average leaf-node visits per query, and the average number of
  `PointAoSoA` groups per visited leaf (the leaf occupancy each partitioner actually achieved).
* A final **query-stream ordering** measurement (using the SAH BVH): the same 500 queries timed in
  generation order versus Morton-sorted order, and the speedup from sorting.

Some things worth noting when reading the output:

* **Leaf visits are the key column.** All four strategies query the *same* points with the *same*
  `pruneTraverse()` code; they differ only in the tree they build. A tighter tree (SAH, midpoint)
  visits far fewer leaves per query than a loose one (Morton), and that leaf-visit count -- not the
  leaf distance math, which is cheap SIMD -- is what dominates query time here. Expect SAH to give
  the fastest queries (fewest leaf visits) and Morton the slowest, with the top-down strategies in
  between. This is the query-time flip side of `Examples/BuildBVH`, which measures the same
  strategies' *build* times: the fast-to-build SFC (Morton) tree is the slowest to query, and the
  slow-to-build SAH tree is the fastest to query.
* **Leaf size is tuned, via `maxLeafGroups` in `main.cpp`.** All four strategies aim for the same
  number of `PointAoSoA` groups per leaf (default 16, i.e. `16 * W` points). This is not the naive
  "one split until fewer than `K` per leaf": a leaf-size sweep on this workload showed queries keep
  getting faster as leaves grow from ~`K` up through ~16-32 groups (fatter leaves mean fewer
  interior nodes to traverse -- fewer SIMD box tests, stack operations, and branch mispredicts --
  while the extra in-leaf `getDistance2()` scans are cheap SIMD), then plateau and slightly regress
  past ~32 as the linear in-leaf scan begins to outweigh the saved traversal. The `Groups/leaf`
  column shows the occupancy each partitioner actually reached (the top-down partitioners' K-way
  splits land somewhat below the target; the Morton chunking hits it almost exactly). Try
  `maxLeafGroups = 8` or `32` to see the shoulders of that curve.
* **Speedup grows with the point count.** At 50,000 points brute force is only tens of
  microseconds, so even the best tree shows a modest (tens-of-×) speedup; the BVH's per-query cost
  grows only logarithmically with the point count while brute force grows linearly, so the gap
  widens dramatically for larger clouds. Raise `numPoints` in `main.cpp` to see this.
* **Query ordering is a free ~1.1-1.5× (batch workloads only).** The query is memory-latency bound
  -- most of its time is dependent node loads down a tree that does not fit in L1/L2 -- so issuing
  spatially-near queries consecutively reuses warm cache. The final section sorts the query batch
  along the same Morton curve used to build the tree and reports the resulting speedup; it grows
  with the point count (the tree gets larger relative to cache). This costs nothing in the data
  structure or the math -- it is purely the *order* you issue queries in -- but applies only when
  you have a batch you are free to reorder, not to a single online query.
* The `PackedBVH`'s branching factor `K` and the SoA group width `W` are independent: `K` governs
  the outer tree's fan-out over groups, while `W` governs how many points live inside one leaf
  group. This example fixes `K = 4` (so tree shape doesn't vary with the compiling machine's SIMD
  tier) but leaves `W` at its ISA-dependent default.
* `PointAoSoA`'s distance queries (`getDistance()`/`getDistance2()`) never touch metadata at all --
  only `getMetaData()` does, and this example calls it only once per query, after the search
  already knows which single group won.
