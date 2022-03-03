This example uses the embedded boundary grid generation from AMReX.
To compile this application, first install AMReX somewhere and point the AMREX_HOME environment variable to it.

Compiling
---------

Compile (with your standard AMReX settings) using

    make -s -j8

Running
-------

With MPI:

    mpirun -np 8 main3d.<something>.ex which_geom=0

To select different geometries, set which_geom to one of the below.

* which_geom = 0 (Airfoil)
* which_geom = 1 (Sphere)
* which_geom = 2 (Dodecahedron)
