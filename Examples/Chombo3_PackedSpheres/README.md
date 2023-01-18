Examples/Chombo3_PackedSpheres
------------------------------

This example uses the embedded boundary grid generation from Chombo3 for constructing a packed spherical bed. 
To compile this application, first install Chombo somewhere and point the CHOMBO_HOME environment variable to it.

Compiling
---------

Compile (with your standard Chombo settings) using

    make -s -j8 DIM=3 main

Running
-------

With MPI:

    mpirun -np 8 main3d.<something>.ex examples.inputs

Some of the geometries will generate cut-cells which AMReX does not support, so the geometries should be run with eb2.cover_multiple_cuts=1.

Input options are

* `bvh = true/false` For turning on/off the BVH accelerator for the CSG operation.
* `n_cells = <integer>` For setting the number of grid cells along the coordinate directions.
* `grid_size = <integer>` For setting the blocking factor.
