Integrations/MPI/OctreeLevelSet
-------------------------------

An adaptive octree level set under MPI, with geometry queries that need no communication.

A coarse grid of root cells covers the box of a model (an `.ebg` file). The roots are sorted by their Morton codes
(`SFC::Morton`) and cut into one contiguous run per rank, as an octree code such as p4est partitions its forest. Every
rank reads the model and compiles it into a `Tape` of its own, so it holds the whole geometry. Each rank then refines
its own roots, cell by cell: a cell is split into eight while the value at its center is, in magnitude, below its
half-diagonal (times a small safety factor), up to a maximum level. In a model with regions (an `overlay`), a cell is
also split where a region changes in it, where one of its eight corners is in another region than its center
(`Tape::region()`), so the interfaces between regions are refined as the surface is. Each leaf stores the signed
distance at its center and, for a model with regions, the region there.

None of this asks another rank anything. The only communication is one reduction at the end, for the summary rank 0
prints: the leaves per level. The program also checks that the ranks' leaves cover the domain exactly once, and exits
with 1 if they do not.

The partition is static: each rank gets the same number of roots, but roots near the surface refine into many more
leaves than the others, so the ranks' work and leaves differ. A code that needs balance repartitions the leaves along
the same Morton order after refining (each rank sends leaves to its neighbors in that order), which this program
leaves out.

Output
------

Every rank writes its leaves as `OctreeLevelSet_<rank>.vtu`, a VTK XML unstructured grid in ASCII: one hexahedron per
leaf, with the cell data `levelset`, `level`, `rank` and, for a model with regions, `region`. Rank 0 writes
`OctreeLevelSet.pvtu`, which lists the pieces. Open `OctreeLevelSet.pvtu` in ParaView or VisIt; a threshold of
`levelset` at 0 shows the object, and coloring by `rank` shows the partition. `make clean` removes the files.

The default model is the classic CSG solid of the `LanguageCSG` example, `Examples/LanguageCSG/solid.ebg`, found by a path
relative to this folder (`../../../Examples/LanguageCSG/solid.ebg`): run the program from here, or give a model as the
first argument, for example `Examples/LanguageDomains/cable/model.ebg`, which has regions.
The number of roots, the maximum level and the safety factor are constants at the top of `main.cpp`; each further
level multiplies the number of leaves near the surface by about four.

This integration is not built or run in continuous integration.

Compiling
---------

An MPI installation is needed (`mpicxx`, `mpirun`; for example Open MPI or MPICH). Either with GNU make:

    make                       # PRECISION=float, ASSERTIONS=ON, EBGEOMETRY_HOME=... are optional

or with CMake (`find_package(MPI)`):

    cmake -S . -B build && cmake --build build      # -DEBGEOMETRY_PRECISION=float is optional

Both put the program in this folder, `OctreeLevelSet.ex`.

Running
-------

    mpirun -n 4 ./OctreeLevelSet.ex
    mpirun -n 4 ./OctreeLevelSet.ex ../../../Examples/LanguageDomains/cable/model.ebg

or `make run NP=4`. It writes its files in the folder it runs in. Add `--oversubscribe` to Open MPI's `mpirun` to run
more ranks than the machine has cores.
