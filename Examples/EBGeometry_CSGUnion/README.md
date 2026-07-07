Examples/EBGeometry_CSGUnion
----------------------------

This folder contains an example of building a CSG *union* of two different kinds
of implicit function: a signed distance field read from a surface mesh, and an
analytic sphere. Both derive from `ImplicitFunction<T>`, so they can be combined
directly. They are merged with a `BVHUnion`, which places the objects in a
bounding volume hierarchy so that closest-object queries traverse the tree
instead of testing every object.

Building
--------

This example is standalone and can be built in three ways. Each needs the path
to the EBGeometry root -- the directory that contains `EBGeometry.hpp` -- which
is two levels up from this folder (`../..`) when building in place.

**CMake**

    cmake -S . -B build
    cmake --build build

The binary is `build/EBGeometry_CSGUnion.ex`. Build in single precision, or against a library
in a different location, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry

**GNU make**

    make

This produces `./EBGeometry_CSGUnion.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o EBGeometry_CSGUnion.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision.

Running
-------

Run from this directory so the default mesh path resolves:

    ./EBGeometry_CSGUnion.ex
    ./EBGeometry_CSGUnion.ex ../../common-3d-test-models/data/cow.obj

With no argument the example loads `cow.obj` from the
`common-3d-test-models` submodule, so make sure it is checked out
(`git submodule update --init`). If you built with CMake the binary is
`build/EBGeometry_CSGUnion.ex` -- still run it from this directory.
