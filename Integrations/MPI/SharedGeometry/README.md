Integrations/MPI/SharedGeometry
-------------------------------

One geometry built once and shared under MPI, with one copy of its memory per node.

* **Build once.** Rank 0 reads a mesh into a `TriMeshSDF` and freezes its pool. The pool's size, its bytes and the
  `TriMeshSDF`'s `Layout` (offsets into the pool, no address) are broadcast, and every rank rebuilds the `TriMeshSDF`
  with its `(Layout, PoolLocation)` constructor instead of building it again.
* **One copy per node.** The ranks of a node (`MPI_Comm_split_type` with `MPI_COMM_TYPE_SHARED`) share an MPI
  shared-memory window (`MPI_Win_allocate_shared`). A `MemoryResource` over the window backs the pool on each of them,
  so only the first rank of each node receives the bytes, and every rank of the node reads that one copy. This works
  because a pool holds offsets, which are valid at whatever address each process maps the window. A node's geometry
  then takes the memory of one rank's, however many ranks run on it.

Every rank then evaluates the same points through the shared copy and compares the values with those of rank 0's own
build. Ranks other than the first of a node receive no bytes, so they can only get the answers through the window.
The program exits with 1 if any value differs.

On GPU nodes, each device still needs a copy of its own: freeze the shared pool and mirror it into device memory with
`Pool::mirror()`, as on one rank.

The default mesh is `Examples/data/dodecahedron.stl`, in the repository; any STL, PLY, VTK or OBJ file can be given
instead.

Compiling
---------

An MPI installation of MPI 3 or later is needed (`mpicxx`, `mpirun`; for example Open MPI or MPICH). Either with GNU
make:

    make                       # PRECISION=float, ASSERTIONS=ON, EBGEOMETRY_HOME=... are optional

or with CMake (`find_package(MPI)`):

    cmake -S . -B build && cmake --build build      # -DEBGEOMETRY_PRECISION=float is optional

Both put the program in this folder, `SharedGeometry.ex`.

Running
-------

    mpirun -n 4 ./SharedGeometry.ex
    mpirun -n 4 ./SharedGeometry.ex ../../../common-3d-test-models/data/armadillo.obj

or `make run NP=4`. Add `--oversubscribe` to Open MPI's `mpirun` to run more ranks than the machine has cores.
