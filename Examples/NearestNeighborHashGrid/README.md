Examples/NearestNeighborHashGrid
--------------------------------

Nearest-neighbor search over a point cloud using the turnkey
[`PointCloudHashGrid`](https://rmrsk.github.io/EBGeometry/doxygen/html/classEBGeometry_1_1PointCloudHashGrid.html)
class -- the uniform-grid counterpart to
[`Examples/NearestNeighborBVH`](../NearestNeighborBVH/README.md), with the **same interface**, so
switching between the tree and the grid is a one-line type change.

`allNearestNeighbors(k)` computes the `k` nearest neighbors of every point in one batched pass; a
self query excludes the point itself, so the neighbors returned are the nearest *other* points. The
grid suits **near-uniform** clouds; for clustered or multi-scale clouds the density-adaptive
`PointCloudBVH` is the better choice.

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

The binary is `NearestNeighborHashGrid.ex`, in this same directory (same as the other two methods
below). Build in single precision, against a library in a different location, or with
`EBGEOMETRY_EXPECT()` runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./NearestNeighborHashGrid.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o NearestNeighborHashGrid.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

Running
-------

    ./NearestNeighborHashGrid.ex

Takes no arguments. It prints the build and per-point query times and one worked `nearestNeighbor()`
result. The neighbor graph is checked against a brute-force scan when built with
`-DEBGEOMETRY_ENABLE_ASSERTIONS`.
