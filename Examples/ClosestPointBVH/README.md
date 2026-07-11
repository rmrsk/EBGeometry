Examples/ClosestPointBVH
------------------------

Closest-point search over a point cloud using the turnkey
[`PointCloudBVH`](https://rmrsk.github.io/EBGeometry/doxygen/html/classEBGeometry_1_1PointCloudBVH.html)
class.

`PointCloudBVH` hides the whole point-cloud search pipeline -- grouping points into SIMD
`PointAoSoA` leaves, building the `PackedBVH`, and running the pruned traversal -- behind a single
constructor and a couple of query methods:

    PointCloudBVH<T> bvh(positions, metadata);   // build once
    bvh.closestPoint(q);                          // nearest point to an arbitrary point q
    bvh.closestPoints(q, k, out);                 // the k nearest, ascending by distance

This is the **external** query form: the query points are arbitrary, not members of the cloud. Its
counterpart, [`Examples/NearestNeighborBVH`](../NearestNeighborBVH/README.md), queries with points
that are already in the cloud (the k-nearest-neighbor graph).

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

Takes no arguments. It prints the build and query times and one worked `closestPoints()` result.
Queries are checked against a brute-force scan when built with `-DEBGEOMETRY_ENABLE_ASSERTIONS`.
