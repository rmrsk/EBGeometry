Integrations/MPI/OctreeLevelSet
-------------------------------

An adaptive octree level set under MPI, with geometry queries that need no communication.

A coarse grid of root cells covers the box of a model (an `.ebg` file). The roots are sorted by their Morton codes
(`SFC::Morton`) and cut into one contiguous run per rank, as an octree code such as p4est partitions its forest. Every
rank reads the model and compiles it into a `Tape` of its own, so it holds the whole geometry. Each rank then refines
its own roots, cell by cell, up to a maximum level. A cell the surface may pass through (the value at its center is, in
magnitude, below its half-diagonal times a small safety factor) is split into eight down to level 2, and below that
while its corners do not predict its center: while the mean of the values at its eight corners (their trilinear
interpolation at the center) misses the value at its center by more than a tolerance times the cell's side. On a flat
part of the surface the two agree and the cell stops; where the surface curves, and most at the model's edges and
corners, they differ and the cells refine further. The test samples only a cell's eight corners and its center, so a
feature that touches none of them (a thin region, or a bump symmetric about the center) can stop a cell early; the
minimum level near the surface keeps such misses to features smaller than a cell of level 2. In a model with regions (an `overlay`), a cell is also split, to
the maximum level, where a region changes in it, where one of its eight corners is in another region than its center
(`Tape::region()`), so the interfaces between regions are refined as the surface is. Each leaf stores the signed
distance at its eight corners and, for a model with regions, the region at its center.

None of this asks another rank anything. The only communication is one reduction at the end, for the summary rank 0
prints: the leaves per level. The program also checks that the ranks' leaves cover the domain exactly once, and exits
with 1 if they do not.

The partition is static: each rank gets the same number of roots, but roots near the surface refine into many more
leaves than the others, so the ranks' work and leaves differ. A code that needs balance repartitions the leaves along
the same Morton order after refining (each rank sends leaves to its neighbors in that order), which this program
leaves out.

Output
------

Every rank writes its leaves as `OctreeLevelSet_<rank>.vtu`, a VTK XML unstructured grid whose arrays are appended as
raw binary (no compression, and no library needed to write it), which VisIt and ParaView read much faster than text: one
hexahedron per leaf, on corners shared with its neighbors (so a reader draws only the outer faces), with the level set
`levelset` at its corners (point data) and the cell data `level`, `rank` and, for a model with regions, `region`. Rank 0
writes `OctreeLevelSet.pvtu`, which lists the pieces. Open `OctreeLevelSet.pvtu` in ParaView or VisIt; a contour of
`levelset` at 0 shows the surface (continuous across leaves of one level, with small cracks where the level changes),
and coloring by `rank` shows the partition. `make clean` removes the files.

The default model is the classic CSG solid of the `LanguageCSG` example, `Examples/LanguageCSG/solid.ebg`, found by a path
relative to this folder (`../../../Examples/LanguageCSG/solid.ebg`): run the program from here, or give a model as the
first argument, for example `Examples/LanguageDomains/cable/model.ebg`, which has regions.
`--max-level N` sets the maximum level (default 5, at most 10), and `--tolerance X` the tolerance, relative to a cell's
side (default 0.01; a smaller one refines more of the curved surface). The leaves a further level adds are only those
where the surface is still not resolved, so the output grows far slower than the four times per level of a uniform
refinement, except at the interfaces between regions, which refine to the maximum level everywhere. The number of
roots, the minimum level near the surface and the safety factor are constants at the top of `main.cpp`.

The program runs on the host only. Each rank refines its roots recursively, cell by cell, asking its tape one point at
a time. The tape can run on a GPU, but this program does not use one: refining on a device would go level by level
instead, evaluating all of a level's cell centers in one `GPU::transform()` call on a mirrored pool, then splitting the
cells that need it.

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
    mpirun -n 4 ./OctreeLevelSet.ex --max-level 7 --tolerance 0.005

or `make run NP=4`. It writes its files in the folder it runs in. Add `--oversubscribe` to Open MPI's `mpirun` to run
more ranks than the machine has cores.
