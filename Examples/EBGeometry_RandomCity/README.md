Examples/EBGeometry_RandomCity
------------------------------

This folder contains a basic example of creating multi-object scenes using EBGeometry.
This example creates a scene consisting of random boxes (buildings), and the scene is created from the union of these buildings.
Two unions are defined:

* A standard union of objects that looks through every object.
* A union that uses BVHs for finding the closest object(s).

This example program illustrates the benefits of using BVHs for closest-object queries.
Rather than computing the distance to all buildings, the BVH allows traversal and a dramatic reduction in algorithmic complexity.

Building
--------

This example is standalone and can be built in three ways. Each needs the path
to the EBGeometry root -- the directory that contains `EBGeometry.hpp` -- which
is two levels up from this folder (`../..`) when building in place.

**CMake**

    cmake -S . -B build
    cmake --build build

The binary is `EBGeometry_RandomCity.ex`, in this same directory (same as the other two methods below). Build in single precision, or against a library
in a different location, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry

**GNU make**

    make

This produces `./EBGeometry_RandomCity.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o EBGeometry_RandomCity.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision.

Running
-------

    ./EBGeometry_RandomCity.ex

