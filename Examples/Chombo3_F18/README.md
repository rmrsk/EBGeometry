Examples/Chombo3_F18
--------------------

This example uses the embedded boundary grid generation from Chombo
This is an advanced example, showing how multiple watertight polygon surfaces can be embedded in a BVH-accelerated CSG union.

This example loads a series of STL files and creates a DCEL mesh from each file.
To speed up the signed distance calculation of each DCEL component, each DCEL mesh is put in a flattened BVH.
To construct the composite implicit function, which is a CSG union, the flattened BVHs are then put inside another BVH.
This creates a BVH-of-BVH type of scene:
The outer BVH provides fast lookup of the "nearest object".
The inner BVHs (stored in the leaf nodes of the outer BVH) provide fast lookup of the nearest triangle. 

Compiling
---------

To compile this application, first install Chombo somewhere and point the CHOMBO_HOME environment variable to it.
Then compile (with your standard Chombo settings) using

    make -s -j8 DIM=3

Compilation should be done in 3D, using C++17 (remember to link with -lstdc++fs).  

Running
-------

With MPI:

    mpirun -np 8 main3d.<something>.ex examples.inputs
