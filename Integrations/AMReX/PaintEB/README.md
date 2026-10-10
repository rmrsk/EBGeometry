Integrations/AMReX/PaintEB
--------------------------

This example uses the embedded boundary grid generation from AMReX and associates each cut cell with the
mesh face closest to it. The implicit function is an EBGeometry `MeshSDF`, and its `getClosestFace()`
gives the face id: the face's index in the mesh, which is its position in the file (less any zero-area
faces the reader removed). Per-face data, such as a boundary condition or a material, can be kept in an
array indexed by that id. The same code runs on CPUs and, in a CUDA or HIP build of AMReX, on GPUs.
To compile this application, first install AMReX somewhere and point the AMREX_HOME environment variable to it.

Compiling
---------

Compile (with your standard AMReX settings) using

    make -s -j8 DIM=3

For a GPU build, add AMReX's usual GPU options, e.g.

    make -s -j8 DIM=3 USE_CUDA=TRUE CUDA_ARCH=80
    make -s -j8 DIM=3 USE_HIP=TRUE AMD_ARCH=gfx90a

SYCL builds of AMReX are not supported, since EBGeometry has no SYCL memory resource yet.

Running
-------

With MPI:

    mpirun -np 8 main3d.<something>.ex eb2.cover_multiple_cuts=1

Some meshes will generate cut-cells which AMReX does not support, so the example should be run with eb2.cover_multiple_cuts=1.

Input options are

* `filename = <string>` The mesh file (STL, PLY, VTK or OBJ). The default is Suzanne from the
  `common-3d-test-models` submodule (`git submodule update --init`), with a path relative to this folder,
  so run the executable from here. The computational domain is sized for that mesh; edit the `RealBox` in
  `main.cpp` for others.
* `n_cell = <integer>` For setting the number of grid cells along the coordinate directions.
* `max_grid_size = <integer>` For setting the blocking factor.
* `num_coarsen_opt = <integer>` For performance tuning the EB generation.

The plot file holds one component, `facet_id`: the id of the face closest to the center of each cut cell,
and -1 in regular and covered cells. EBGeometry's signed distance is negative inside the mesh, and AMReX
treats negative values as fluid, so the fluid region is the interior of the mesh.

How it works
------------

The mesh and its BVH are read into a host `EBGeometry::Pool` as a `MeshSDF`. In a GPU build the pool is
frozen and mirrored to device memory in one piece, and the `MeshSDF` is rebased onto the mirror. The AMReX
implicit function derives from `amrex::GPUable` and holds both descriptors (host and device); each
compilation pass uses its own. The same functor is captured by value in the `amrex::ParallelFor` that
paints the cut cells. The pools are owned by `main()`, which clears AMReX's EB index space before they go
out of scope.
