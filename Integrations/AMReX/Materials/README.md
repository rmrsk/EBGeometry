Integrations/AMReX/Materials
----------------------------

Which part of an object each cut cell lies on. The armadillo with a cylinder bored through its chest
(`Difference`), compiled into a `Tape`, is handed to AMReX's EB2 grid generation, and each cut cell is
labeled twice:

* with the tape's leaf closest to it (`closestPrimitive()`), in expression order: the armadillo 0, the bore 1;
* on the armadillo, with the id of its closest mesh face (`getClosestFace()`, asked at the point in the
  mesh's own frame that `closestPrimitive()` returns).

They are the indices into tables of your own, of per-part and per-face data such as boundary conditions or
materials. It mirrors the [`Materials`](../../../Examples/Materials/README.md) example.

Where several faces are equally close (the closest point is an edge or a vertex they share), each of them is
the closest face, and a CPU and a GPU run may name different ones.

The same code runs on CPUs and, in a CUDA or HIP build of AMReX, on GPUs: the geometry's pool is mirrored to
device memory in one piece, and the implicit function holds a view of each copy.

The default mesh comes from the `common-3d-test-models` git submodule: clone the repository with
`--recurse-submodules`, or run `git submodule update --init`.

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
* `filename = <string>`: the mesh file, relative to this folder.

It writes a plot file, `plt`, with each cut cell's leaf in `leaf` and its closest face in `face_id` (-1 elsewhere,
and in `face_id` on the bore).
