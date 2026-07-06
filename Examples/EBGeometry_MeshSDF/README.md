Examples/EBGeometry_MeshSDF
---------------------------

This folder contains a basic example of using EBGeometry, with three different
representations of the signed distance field.

* A naive approach which iterates through all facets and computes the signed distance.
* Using a conventional bounding volume hierarchy with (references to) primitives stored directly in the nodes.
* A flattened, more compact bounding volume hierarchy.

Note that SDF queries have different complexity for different geometries and input points.
For example, a tessellated sphere has a "blind spot" in its center where even BVHs will visit most, if not all, primitives.

Building
--------

This example is standalone and can be built in three ways. Each needs the path
to the EBGeometry root -- the directory that contains `EBGeometry.hpp` -- which
is two levels up from this folder (`../..`) when building in place.

**CMake**

    cmake -S . -B build
    cmake --build build

The binary is `build/EBGeometry_MeshSDF`. Build in single precision, or against a library
in a different location, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry

**GNU make**

    make

This produces `./EBGeometry_MeshSDF`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o EBGeometry_MeshSDF

Add `-DEBGEOMETRY_PRECISION=float` for single precision.

Running
-------

Run from this directory so the default mesh path resolves:

    ./EBGeometry_MeshSDF
    ./EBGeometry_MeshSDF ../../common-3d-test-models/data/cow.obj

With no argument the example loads `armadillo.obj` from the
`common-3d-test-models` submodule, so make sure it is checked out
(`git submodule update --init`). If you built with CMake the binary is
`build/EBGeometry_MeshSDF` -- still run it from this directory.
