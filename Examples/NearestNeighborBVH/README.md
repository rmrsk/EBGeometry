Examples/NearestNeighborBVH
---------------------------

Nearest-neighbor search over a point cloud using the turnkey
[`PointCloudBVH`](https://rmrsk.github.io/EBGeometry/doxygen/html/classEBGeometry_1_1PointCloudBVH.html)
class (see [EBGeometry issue #92](https://github.com/rmrsk/EBGeometry/issues/92)).

Where [`Examples/ClosestPointBVH`](../ClosestPointBVH/README.md) queries with arbitrary *external* points,
this example queries with points that are **already in the cloud** -- the classic
k-nearest-neighbor graph. The whole pipeline is hidden behind the constructor and one call:

    PointCloudBVH<T> bvh(positions, metadata);       // build once
    bvh.nearestNeighbor(i);                            // nearest OTHER particle to particle i
    auto graph = bvh.allNearestNeighbors(kNN);         // kNN nearest of EVERY particle, batched

500,000 random points in the unit cube are built into a `PointCloudBVH`, then
`allNearestNeighbors(kNN)` computes the `kNN` nearest neighbors of every point in one batched pass.
`kNN` is fixed at 1 in `main.cpp`; raise it for a wider neighbor graph. A spread sample of the result
is checked against a brute-force scan.

A self query excludes the point itself (otherwise it would always find itself at distance zero); the
returned neighbors are the nearest *other* points. Everything stays in **squared distance**
(`Hit::distanceSquared`) on the hot path; the example only takes a square root when printing. The
branching factor `K` and SoA leaf width `W` are `PointCloudBVH` template parameters that default to
the SIMD-optimal values for the precision `T`, so this example does not name them.

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

The binary is `NearestNeighborBVH.ex`, in this same directory (same as the other two methods below).
Build in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()`
runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./NearestNeighborBVH.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o NearestNeighborBVH.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

Running
-------

    ./NearestNeighborBVH.ex

Takes no arguments. It generates a random 500,000-point cloud (fixed seed, so results are
reproducible on a given machine) and prints the build and per-point query times and one worked
`nearestNeighbor()` result. A spread sample of the neighbor graph is checked against a brute-force
scan with `EBGEOMETRY_EXPECT`, so building with `-DEBGEOMETRY_ENABLE_ASSERTIONS` aborts on any
mismatch; the checks compile out otherwise.
