Examples/EBGeometry_OctreeBoundingVolume
----------------------------------------

This folder contains a basic example of using EBGeometry for computing an approximate bounding volume for an implicit function.

Building
--------

This example is standalone and can be built in three ways. Each needs the path
to the EBGeometry root -- the directory that contains `EBGeometry.hpp` -- which
is two levels up from this folder (`../..`) when building in place.

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

