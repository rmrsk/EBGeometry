This example uses the embedded boundary grid generation from AMReX for constructing a random urban city environment (consiting of boxes).
To compile this application, first install AMReX somewhere and point the AMREX_HOME environment variable to it.

Compiling
---------

Compile (with your standard AMReX settings) using

    make -s -j8 DIM=3

Running
-------

With MPI:

    mpirun -np 8 main3d.<something>.ex eb2.cover_multiple_cuts=1

Available input options are

* `bvh = true/false` For turning on/off the BVH accelerator for the CSG operation.
* `n_cell = <integer>` For setting the number of grid cells along the coordinate directions.
* `max_grid_size = <integer>` For setting the blocking factor.
* `num_coarsen_opt = <integer>` For performance tuning the EB generation.
