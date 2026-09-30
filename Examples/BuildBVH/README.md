Examples/BuildBVH
-----------------

A benchmark of EBGeometry's BVH construction strategies. It builds a BVH over a random point cloud
with every `BVH::Strategy` -- top-down binned SAH, Centroid (equal-count) and Midpoint splits,
ClusterSAH, and a space-filling-curve build along each of the Morton, Nested and Hilbert curves --
each with at most four points per leaf, and for each one times

* `BVH::buildTopology()` alone: the shape of the tree, before any primitive is stored;
* the full `PackedBVH` construction: the shape plus the primitives, copied into leaf order;
* a fixed query workload: the closest cloud point to each of a few thousand random query points,
  found with `PackedBVH::pruneTraverse()`.

Every strategy must find the same closest points. The example checks that, and a sample of the
queries against a brute-force scan, and exits with an error if any disagree.

Building
--------

This example is standalone and can be built in three ways. Each needs the path
to the EBGeometry root -- the directory that contains `EBGeometry.hpp` -- which
is two levels up from this folder (`../..`) when building in place. See
[Direct compilation](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildingdirectcompile),
[Building with GNU Make](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildinggnumake), and
[Building with CMake](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildingcmake) in the user
documentation for more detail on each approach.

**CMake**

    cmake -S . -B build
    cmake --build build

The binary is `BuildBVH.ex`, in this same directory (same as the other two methods below). Build in single precision, against a library
in a different location, or with `EBGEOMETRY_EXPECT()` runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./BuildBVH.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o BuildBVH.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

Running
-------

    ./BuildBVH.ex

Takes no arguments. It prints one row per strategy: the `buildTopology()` time and the full
`PackedBVH` construction time (which includes a `buildTopology()` of its own), the depth and node
count of the tree, and the time for all the closest-point queries, all in seconds. A fast build is
not the same as a fast tree: compare the build columns against the query column for the trade-off
between them. Build in Release (`-O3`) for timings that mean anything; a Debug build runs the same
workload in a few seconds.
