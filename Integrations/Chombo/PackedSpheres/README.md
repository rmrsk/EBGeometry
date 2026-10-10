Integrations/Chombo/PackedSpheres
---------------------------------

This example uses the embedded boundary grid generation from Chombo3 for constructing a packed bed of 8000 randomly sized spheres, blended together with a smooth union. The
implicit function is an EBGeometry `BVHSmoothUnion`, accelerated by a packed bounding volume hierarchy.
To compile this application, first install Chombo somewhere and point the CHOMBO_HOME environment variable to it.

Compiling
---------

Compile (with your standard Chombo settings) using

    make -s -j8 DIM=3 CXXSTD=17 main

EBGeometry needs C++17, and Chombo compiles as C++11 unless told otherwise: add `CXXSTD=17` to the
command line, or `CXXSTD = 17` to Chombo's `Make.defs.local`.

Running
-------

With MPI:

    mpirun -np 8 main3d.<something>.ex examples.inputs

Input options are

* `bvh = true/false` Use the BVH (the default), or visit every sphere on each query. Both give the
  same geometry; only the time differs.
* `n_cells = <integer>` For setting the number of grid cells along the coordinate directions.
* `grid_size = <integer>` For setting the blocking factor.

The union and the BVH over the spheres are built into an `EBGeometry::Pool`. Chombo copies implicit
functions, so the implicit function holds the union by value and the pool by `shared_ptr`: every copy
shares the one pool, which lives as long as the last of them. EBGeometry's signed distance is negative
inside the spheres; the implicit function flips the sign so that the fluid is the space around them.
