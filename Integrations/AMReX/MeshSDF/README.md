Integrations/AMReX/MeshSDF
--------------------------

This example uses the embedded boundary grid generation from AMReX, with the implicit function given by
an EBGeometry `TriMeshSDF`: a surface mesh (STL, PLY, VTK or OBJ; polygon faces are triangulated)
accelerated by a packed bounding volume hierarchy. The same code runs on CPUs and, in a CUDA or HIP build
of AMReX, on GPUs.
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

* `filename = <string>` The mesh file. The default is the armadillo from the `common-3d-test-models`
  submodule (`git submodule update --init`), with a path relative to this folder, so run the executable
  from here. The computational domain is sized for that mesh; edit the `RealBox` in `main.cpp` for others.
* `n_cell = <integer>` For setting the number of grid cells along the coordinate directions.
* `max_grid_size = <integer>` For setting the blocking factor.
* `num_coarsen_opt = <integer>` For performance tuning the EB generation.

EBGeometry's signed distance is negative inside the mesh, and AMReX treats negative values as fluid, so
the fluid region is the interior of the mesh.

How it works
------------

The mesh and its BVH are read into a host `EBGeometry::Pool`. In a GPU build the pool is frozen and
mirrored to device memory in one piece, and the `TriMeshSDF` is rebased onto the mirror. The AMReX
implicit function derives from `amrex::GPUable` and holds both descriptors (host and device), because
AMReX evaluates it inside GPU kernels but may also call it on the host; each compilation pass uses its
own. The BVH branching factor and SoA width are given explicitly so that the host and device
compilation passes agree on the `TriMeshSDF` type. The pools are owned by `main()`, which clears
AMReX's EB index space before they go out of scope.
