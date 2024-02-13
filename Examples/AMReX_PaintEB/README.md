Examples/AMReX_PaintEB
----------------------

This example uses the embedded boundary grid generation from AMReX and associates triangle meta-data with AMReX-based cut-cells.
To compile this application, first install AMReX somewhere and point the AMREX_HOME environment variable to it.

Compiling
---------

Compile (with your standard AMReX settings) using

    make -s -j8 DIM=3

Running
-------

With MPI:

    mpirun -np 8 main3d.<something>.ex eb2.cover_multiple_cuts=1 which_geom=0

Some of the geometries will generate cut-cells which AMReX does not support, so the geometries should be run with eb2.cover_multiple_cuts=1.

Other input options are

* `n_cell = <integer>` For setting the number of grid cells along the coordinate directions.
* `max_grid_size = <integer>` For setting the blocking factor.
* `num_coarsen_opt = <integer>` For performance tuning the EB generation.

Supported geometries
--------------------
To select different geometries, set which_geom to one of the below.

* `which_geom = 0` Airfoil
* `which_geom = 1` Sphere
* `which_geom = 2` Dodecahedron
* `which_geom = 3` Horse
* `which_geom = 4` Porsche
* `which_geom = 5` Orion capsule
* `which_geom = 6` Armadillo
* `which_geom = 7` Adirondack
