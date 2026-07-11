Examples/NearestNeighborHashGrid
--------------------------------

Nearest-neighbor search over a point cloud using the turnkey
[`PointCloudHashGrid`](https://rmrsk.github.io/EBGeometry/doxygen/html/classEBGeometry_1_1PointCloudHashGrid.html)
class -- the uniform-grid counterpart to
[`Examples/NearestNeighborBVH`](../NearestNeighborBVH/README.md).

`PointCloudHashGrid` answers the same queries as `PointCloudBVH` and exposes the **same interface**
(same `Hit`, same methods), so switching between the tree and the grid is a one-line type change:

    PointCloudHashGrid<T> grid(positions, metadata);   // build once (counting-sort into cells)
    grid.nearestNeighbor(i);                            // nearest OTHER particle to particle i
    auto graph = grid.allNearestNeighbors(kNN);         // kNN nearest of EVERY particle, batched

500,000 random points in the unit cube are counting-sorted into a uniform grid, then
`allNearestNeighbors(kNN)` computes the `kNN` nearest neighbors of every point in one batched,
spatially-ordered pass. `kNN` is fixed at 1 in `main.cpp`; raise it for a wider neighbor graph. A
spread sample of the result is checked against the class's own `O(N)` brute-force reference, which
also times the baseline for a fair per-point speedup (a full N² all-pairs scan is infeasible at this
size).

**Grid vs. tree.** The grid stores points in a dense array of fixed-size cells and answers a query by
an expanding-shell search outward from the query point's cell, stopping (exactly, never missing a
neighbor) once the best distance is closer than any unvisited cell can hold. For a **near-uniform**
cloud this builds and queries faster than the BVH — an O(N) counting-sort build with no tree, and a
query that touches only a handful of cells. For a **clustered / multi-scale** cloud, though, a single
global cell size is a poor fit and `PointCloudBVH`'s density-adaptive tree is the better choice. The
grid also serves only point queries; it is not a BVH and cannot be composed into an outer BVH/CSG.

A self query excludes the point itself (otherwise it would always find itself at distance zero); the
returned neighbors are the nearest *other* points. Everything stays in **squared distance**
(`Hit::distanceSquared`) on the hot path; the example only takes a square root when printing.

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

Takes no arguments. It generates a random 500,000-point cloud (fixed seed, so results are
reproducible on a given machine) and prints the build time, the brute-force and `PointCloudHashGrid`
per-point times with the speedup, and one worked `nearestNeighbor()` result. A spread sample of the
neighbor graph is checked against a brute-force scan with `EBGEOMETRY_EXPECT`, so building with
`-DEBGEOMETRY_ENABLE_ASSERTIONS` aborts on any mismatch; the checks compile out otherwise.
