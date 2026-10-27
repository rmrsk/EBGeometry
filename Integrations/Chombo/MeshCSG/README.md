Integrations/Chombo/MeshCSG
---------------------------

A mesh composed with analytic shapes: the xyzrgb dragon scaled onto a table and blended into it by a `SmoothUnion`, compiled into a `Tape` and handed to Chombo's `GeometryShop` as an implicit function. It mirrors the
[`MeshCSG`](../../../Examples/MeshCSG/README.md) example.

The default mesh comes from the `common-3d-test-models` git submodule: clone the repository with
`--recurse-submodules`, or run `git submodule update --init`.

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

* `n_cells = <integer>`: cells along each direction of the cubic domain around the object (default 128).
* `grid_size = <integer>`: the box size (default 16).
* `filename = <string>`: the mesh file, relative to this folder.

It writes the implicit function's value in every cell to `example.hdf5`.
