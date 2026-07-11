Examples/BuildBVH
-----------------

A benchmark of EBGeometry's BVH construction strategies. It builds a BVH over a random point cloud
with each available strategy -- top-down (centroid, SAH, and midpoint partitioners), bottom-up along
the Morton, Nested, and Hilbert space-filling curves, and ClusterSAH -- and reports the build time
for each.

For most strategies it times two ways of reaching a queryable `PackedBVH`: the traditional
`TreeBVH`-then-`pack()` path, and `PackedBVH`'s direct constructor, which packs the primitives
straight into the flat, queryable layout without building a `TreeBVH` first.

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

Takes no arguments. It prints a table of build times, in seconds, for every strategy (`TreeBVH`,
`+ pack()`, and their `Total`, alongside the direct-constructor `Direct build` time). This times
*build* only, not query performance or tree quality.
