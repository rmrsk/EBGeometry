Integrations/MPI/ParticleWalls
------------------------------

A domain-decomposed particle code under MPI. The domain is a box around an object, cut into slabs along x, one per
rank, and each rank owns the particles in its slab. Every rank reads the whole object into a `TriMeshSDF` of its own,
so it traces the paths of its particles without asking another rank.

Each step moves every particle from x to x + v dt, reflecting it off the object where its path first crosses the
surface (`firstHit()` on the segment) and off the walls of the box, and then sends the particles that left a rank's
slab to the rank that owns them now (`MPI_Alltoallv`). That migration, which a particle code does anyway, is the only
communication: the geometry queries need none.

Rank 0 prints the number of particles, bounces and migrations every 50 steps, and at the end how many particles
there are and how many are inside the object. The program exits with 1 if the migration lost a particle.

The default mesh is `Examples/data/dodecahedron.stl`, in the repository; any STL, PLY, VTK or OBJ file can be given
instead.

Compiling
---------

An MPI installation is needed (`mpicxx`, `mpirun`; for example Open MPI or MPICH). Either with GNU make:

    make                       # PRECISION=float, ASSERTIONS=ON, EBGEOMETRY_HOME=... are optional

or with CMake (`find_package(MPI)`):

    cmake -S . -B build && cmake --build build      # -DEBGEOMETRY_PRECISION=float is optional

Both put the program in this folder, `ParticleWalls.ex`.

Running
-------

    mpirun -n 4 ./ParticleWalls.ex
    mpirun -n 4 ./ParticleWalls.ex ../../../common-3d-test-models/data/armadillo.obj

or `make run NP=4`. Add `--oversubscribe` to Open MPI's `mpirun` to run more ranks than the machine has cores.
