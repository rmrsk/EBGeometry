Integrations/Chombo/RandomCity
------------------------------

A city of 20 x 20 randomly sized buildings (`BoxSDF`) in a `BVHUnion`, handed to Chombo's `GeometryShop` as an implicit function. It mirrors the
[`ManyObjects`](../../../Examples/ManyObjects/README.md) example.

EBGeometry's value is negative inside the object and the platform reads negative values as fluid, so the
implicit function flips the sign: the fluid is around the object.

Compiling
---------

Install Chombo 3 with C++17 (`CXXSTD = 17` in `Make.defs.local`) and point `CHOMBO_HOME` to its `lib` folder, then:

    make -s -j8 DIM=3 OPT=HIGH main

Running
-------

    mpirun -np 8 ./main3d.<something>.ex examples.inputs

Options (in `examples.inputs`):

* `n_cells = <integer>`: cells along the longest side of the domain around the object, whose cells are cubes (default 128).
* `grid_size = <integer>`: the box size (default 16).
* `lots = <integer>`: lots along each side of the city (default 20).

It writes the implicit function's value in every cell to `example.hdf5`.
