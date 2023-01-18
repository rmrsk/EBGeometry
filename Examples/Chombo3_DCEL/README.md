Examples/Chombo3_DCEL
---------------------

This example uses the embedded boundary grid generation from Chombo3, and shows how to use DCEL functionality from EBGeometry. 
To compile this application, first install Chombo somewhere and point the CHOMBO_HOME environment variable to it.

Compiling
---------

Compile (with your standard Chombo settings) using

    make -s -j8 DIM=3 main

Running
-------

With MPI:

    mpirun -np 8 main3d.<something>.ex examples.inputs

Other input options are

* `n_cells = <integer>` For setting the number of grid cells along the coordinate directions.
* `grid_size = <integer>` For setting the blocking factor.
* `which_geom = <integer>` For setting the geometry.

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
