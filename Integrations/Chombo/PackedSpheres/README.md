Integrations/Chombo/PackedSpheres
---------------------------------

3,000 spheres of random sizes at random places in a cylinder (centres by rejection sampling), joined by a `BVHSmoothUnion` and handed to Chombo's `GeometryShop` as an implicit function: the fluid is the space between the spheres. It mirrors the
[`PackedSpheres`](../../../Examples/PackedSpheres/README.md) example.

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
* `num_spheres = <integer>`: the number of spheres (default 3000).

It writes the implicit function's value in every cell to `example.hdf5`.
