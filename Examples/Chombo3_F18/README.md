This example uses the embedded boundary grid generation from AMReX.
This is an advanced example, showing how multiple files (each containing a watertight polygon) can be embedded in a BVH-accelerated CSG union.
To compile this application, first install Chombo somewhere and point the CHOMBO_HOME environment variable to it.

Compiling
---------

Compile (with your standard Chombo settings) using

    make -s -j8

Compilation should be done in 3D, using C++17 (remember to link with -lstdc++fs).  

Running
-------

With MPI:

    mpirun -np 8 main3d.<something>.ex examples.inputs
