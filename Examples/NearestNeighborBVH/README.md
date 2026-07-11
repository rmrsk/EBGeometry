Examples/NearestNeighborBVH
---------------------------

Nearest-neighbor search over a point cloud using the turnkey
[`PointCloudBVH`](https://rmrsk.github.io/EBGeometry/doxygen/html/classEBGeometry_1_1PointCloudBVH.html)
class.

Where [`Examples/ClosestPointBVH`](../ClosestPointBVH/README.md) queries with arbitrary *external*
points, this example queries with points that are **already in the cloud** -- the classic
k-nearest-neighbor graph. `allNearestNeighbors(k)` computes the `k` nearest neighbors of every point
in one batched pass; a self query excludes the point itself, so the neighbors returned are the
nearest *other* points.

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

Takes no arguments. It prints the build and per-point query times and one worked `nearestNeighbor()`
result. The neighbor graph is checked against a brute-force scan when built with
`-DEBGEOMETRY_ENABLE_ASSERTIONS`.
