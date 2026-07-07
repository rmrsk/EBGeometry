Examples/EBGeometry_OctreeBoundingVolume
----------------------------------------

This folder shows how to automatically find a tight bounding box around a shape whose exact
extent isn't known in advance.

Simple shapes like a sphere or a box have an obvious bounding box, but once you start combining
and transforming shapes -- translating, scaling, smoothing, rotating -- the result's exact
extent is no longer obvious from its formula alone. This example builds such a shape (a sphere
that is translated, scaled, slightly smoothed to round off any sharp features, and rotated), then
finds a tight box around it *numerically* rather than analytically: starting from a large, coarse
box known to contain the shape, it recursively splits each box into eight equal sub-boxes (an
*octree*), discards the ones that don't touch the shape's surface, and keeps refining until it
reaches a target resolution. The result is a box that hugs the shape closely, without ever having
derived its extent algebraically.

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

The binary is `EBGeometry_OctreeBoundingVolume.ex`, in this same directory (same as the other two methods below). Build in single precision, or against a library
in a different location, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry

**GNU make**

    make

This produces `./EBGeometry_OctreeBoundingVolume.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o EBGeometry_OctreeBoundingVolume.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision.

Running
-------

    ./EBGeometry_OctreeBoundingVolume.ex

This example takes no arguments. It prints the resulting approximate bounding box (its low and
high corners) for the transformed sphere. The search starts from a fixed, deliberately loose
box of $\pm 10$ in each direction and refines for 8 octree levels; increasing the level count in
`main.cpp` tightens the result further, at the cost of more subdivision work.
