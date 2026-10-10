Integrations/AMReX/RandomCity
-----------------------------

This example uses the embedded boundary grid generation from AMReX for constructing a random urban environment of 2500 buildings (boxes). The
implicit function is an EBGeometry `BVHUnion`, accelerated by a packed bounding volume hierarchy. The same
code runs on CPUs and, in a CUDA or HIP build of AMReX, on GPUs.
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

Available input options are

* `bvh = true/false` Use the BVH (the default), or visit every building on each query.
* `n_cell = <integer>` For setting the number of grid cells along the coordinate directions.
* `max_grid_size = <integer>` For setting the blocking factor.
* `num_coarsen_opt = <integer>` For performance tuning the EB generation.

EBGeometry's signed distance is negative inside the buildings, and AMReX treats negative values as fluid, so the
implicit function flips the sign: the fluid is the space around the buildings.

How it works
------------

The buildings and the BVH over them are built into a host `EBGeometry::Pool` as a `BVHUnion`, a plain
value type. In a GPU build the pool is frozen and mirrored to device memory in one piece, and the union is
rebased onto the mirror. The AMReX implicit function derives from `amrex::GPUable` and holds both
descriptors (host and device), because AMReX evaluates it inside GPU kernels but may also call it on the
host; each compilation pass uses its own. The pools are owned by `main()`, which clears AMReX's EB index
space before they go out of scope.

With `bvh = false` the implicit function visits every building on each
query instead of letting the BVH skip the distant ones. Both give the same geometry; only the time differs.
