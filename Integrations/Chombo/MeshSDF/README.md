Integrations/Chombo/MeshSDF
---------------------------

This example uses the embedded boundary grid generation from Chombo3, loading a surface mesh as an
EBGeometry `TriMeshSDF` (a triangle mesh accelerated by a packed bounding volume hierarchy; polygon faces
are triangulated).
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

* `filename = <string>` The mesh file (STL, PLY, VTK or OBJ). The default is the armadillo from the
  `common-3d-test-models` submodule (`git submodule update --init`), with a path relative to this folder,
  so run the executable from here.
* `domain_lo = <real>`, `domain_hi = <real>` The computational domain, a cube from `domain_lo` to
  `domain_hi` along every axis. The defaults, -125 and 125, hold the armadillo.
* `n_cells = <integer>` For setting the number of grid cells along the coordinate directions.
* `grid_size = <integer>` For setting the blocking factor.

The mesh and its BVH are read into an `EBGeometry::Pool`. Chombo copies implicit functions, so the
implicit function holds the `TriMeshSDF` by value and the pool by `shared_ptr`: every copy shares the one
pool, which lives as long as the last of them. EBGeometry's signed distance is negative inside the mesh;
the implicit function flips the sign so that the fluid is outside the mesh.
