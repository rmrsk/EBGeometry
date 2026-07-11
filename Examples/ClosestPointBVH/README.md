Examples/ClosestPointBVH
------------------------

Closest-point search over a point cloud using the turnkey
[`PointCloudBVH`](https://rmrsk.github.io/EBGeometry/doxygen/html/classEBGeometry_1_1PointCloudBVH.html)
class (see [EBGeometry issue #92](https://github.com/rmrsk/EBGeometry/issues/92)).

`PointCloudBVH` hides the whole point-cloud search pipeline -- grouping points into SIMD
`PointAoSoA` leaves, building the `PackedBVH`, and running the pruned traversal -- behind a single
constructor and a couple of query methods:

    PointCloudBVH<T> bvh(positions, metadata);   // build once
    bvh.closestPoint(q);                          // nearest particle to an arbitrary point q
    bvh.closestPoints(q, k, out);                 // the k nearest, ascending by distance

500,000 random points in the unit cube are built into a `PointCloudBVH`, then 500 arbitrary query
points are resolved with `closestPoint()` and checked against a brute-force scan. It then
demonstrates the k-nearest form `closestPoints()` on one query point.

This is the **external** query form: the query points are arbitrary, not members of the cloud. Its
counterpart, [`Examples/NearestNeighborBVH`](../NearestNeighborBVH/README.md), queries with points that
are already in the cloud (the k-nearest-neighbor graph).

Everything stays in **squared distance** (`Hit::distanceSquared`) on the hot path -- there is no
`sqrt` in the traversal; the example only takes a square root when it prints a human-readable
distance. The branching factor `K` and the SoA leaf width `W` are `PointCloudBVH` template
parameters that default to the SIMD-optimal values for the precision `T`, so this example does not
name them.

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

The binary is `ClosestPointBVH.ex`, in this same directory (same as the other two methods below). Build
in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()` runtime
assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./ClosestPointBVH.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o ClosestPointBVH.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

Running
-------

    ./ClosestPointBVH.ex

Takes no arguments. It generates a random 500,000-point cloud and 500 query points (fixed seeds, so
results are reproducible on a given machine) and prints the build and query times and one worked
`closestPoints()` result. Every query is checked against a brute-force scan with `EBGEOMETRY_EXPECT`,
so building with `-DEBGEOMETRY_ENABLE_ASSERTIONS` aborts on any mismatch; the checks compile out
otherwise.
