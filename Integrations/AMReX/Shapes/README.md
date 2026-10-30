Integrations/AMReX/Shapes
-------------------------

Analytic shapes handed to AMReX's EB2 grid generation as an implicit function: one shape, a torus (the least a
coupling needs), or the classic CSG solid, a cube rounded by a ball and drilled along each axis, compiled into
a `Tape` or composed at compile time with the fused operations (`Fused::`). It mirrors the
[`Shapes`](../../../Examples/Shapes/README.md), [`BasicCSG`](../../../Examples/BasicCSG/README.md) and
[`FusedCSG`](../../../Examples/FusedCSG/README.md) examples.

The same code runs on CPUs and, in a CUDA or HIP build of AMReX, on GPUs. A shape, and a fused composition of shapes, is a plain
value, so nothing has to be copied to the device; a tape lives in a pool, which is mirrored to device memory in one
piece, and the implicit function holds a view of each copy.

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

* `n_cell = <integer>`: cells along the longest side of the domain around the object, whose cells are cubes (default 128).
* `max_grid_size = <integer>`: the largest box (default 32).
* `shape = <torus|csg|fused>`: the object (default `csg`).

It writes a plot file, `plt`.
