Examples/AMReX_Shapes
---------------------

This example uses the embedded boundary grid generation from AMReX, using analytic SDFs from EBGeometry. 
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

* `which_geom = 0` Sphere
* `which_geom = 1` Plane
* `which_geom = 2` Infinite cylinder
* `which_geom = 3` Finite cylinder
* `which_geom = 4` Capsule
* `which_geom = 5` Box
* `which_geom = 6` Rounded box
* `which_geom = 7` Torus
* `which_geom = 8` Infinite cone
* `which_geom = 9` Finite cone
* `which_geom = 10` Spherical shell
* `which_geom = 11` Death star
* `which_geom = 12` Rounded box
* `which_geom = 13` Kern Perlin's gradient noise SDF
