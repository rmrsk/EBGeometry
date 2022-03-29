This example uses the embedded boundary grid generation from Chombo3, and shows how to use basic SDFs from EBGeometry. 
To compile this application, first install Chombo somewhere and point the CHOMBO_HOME environment variable to it.

Compiling
---------

Compile (with your standard Chombo settings) using

    make -s -j8 DIM=3 OPT=HIGH main

Running
-------

With MPI:

    mpirun -np 8 main3d.<something>.ex examples.inputs

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
