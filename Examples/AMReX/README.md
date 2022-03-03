This example uses the embedded boundary grid generation from AMReX.
To compile this application, first install AMReX somewhere and point the AMREX_HOME environment variable to it.

Compile and run this application (with MPI) by

    make -s -j8
    mpirun -np 8 main3d.<something>.ex

To select different geometries, put

* Airfoil geometry:

      mpirun -np 8 main3d.<something>.ex which_geom=0

* Sphere geometry:

      mpirun -np 8 main3d.<something>.ex which_geom=1

* Car geometry:

      mpirun -np 8 main3d.<something>.ex which_geom=2

