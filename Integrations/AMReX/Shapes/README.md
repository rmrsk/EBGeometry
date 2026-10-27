Integrations/AMReX/Shapes
-------------------------

One analytic shape, a torus, handed to AMReX's EB2 grid generation as an implicit function: the least a coupling needs. It mirrors the
[`Shapes`](../../../Examples/Shapes/README.md) example.

The same code runs on CPUs and, in a CUDA or HIP build of AMReX, on GPUs: a shape is a plain value, so
nothing has to be copied to the device.

EBGeometry's value is negative inside the object and the platform reads negative values as fluid, so the
implicit function flips the sign: the fluid is around the object.

Compiling
---------

Install AMReX and point `AMREX_HOME` to it, then:

    make -s -j8 DIM=3
    make -s -j8 DIM=3 USE_CUDA=TRUE CUDA_ARCH=80      # or USE_HIP=TRUE AMD_ARCH=gfx90a

SYCL builds of AMReX are rejected at compile time: EBGeometry has no SYCL memory resource yet.

Running
-------

    mpirun -np 8 ./main3d.<something>.ex

Options (on the command line or in an inputs file):

* `n_cell = <integer>`: cells along each direction of the cubic domain around the object (default 128).
* `max_grid_size = <integer>`: the largest box (default 32).

It writes a plot file, `plt`.
