Examples/EBGeometry_Shapes
--------------------------

This folder contains a very basic example of using EBGeometry for creating analytic signed distance fields.

Building
--------

This example is standalone and can be built in three ways. Each needs the path
to the EBGeometry root -- the directory that contains `EBGeometry.hpp` -- which
is two levels up from this folder (`../..`) when building in place.

**CMake**

    cmake -S . -B build
    cmake --build build

The binary is `build/EBGeometry_Shapes`. Build in single precision, or against a library
in a different location, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry

**GNU make**

    make

This produces `./EBGeometry_Shapes`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o EBGeometry_Shapes

Add `-DEBGEOMETRY_PRECISION=float` for single precision.

Running
-------

    ./EBGeometry_Shapes

If you built with CMake the binary is `build/EBGeometry_Shapes`.
