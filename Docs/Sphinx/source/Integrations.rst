.. _Chap:Integrations:

Integrations
============

The integrations show how EBGeometry is used with third-party code, under :file:`Integrations/`. They are of two
kinds:

* :file:`Integrations/MPI` uses EBGeometry in a parallel code under MPI (:ref:`Sec:ParallelUse`). It needs only an
  MPI installation.
* :file:`Integrations/AMReX` couples EBGeometry to the embedded-boundary grid generation of AMReX
  :cite:`AMReX2019` (:ref:`Sec:IntegrationsAMReX`).

.. important::

   The integrations are not built as part of EBGeometry's continuous integration: unlike everything under
   :ref:`Chap:Examples`, which is compiled and run on every pull request, a change to the third-party code (or to
   EBGeometry itself) can break them without anyone noticing. Treat them as a starting point for your own
   integration.

.. _Sec:ParallelUse:

Parallel use with MPI
---------------------

EBGeometry contains no MPI, and needs none. It is built for codes in which **every rank holds the whole geometry**:
a signed distance, a ray, a closest face or a region is then answered by the rank that asks, from its own copy, with
no communication. A domain-decomposed code keeps its own decomposition (of cells, of particles) and communicates
what it does anyway, such as ghost cells or particles that move to another rank; the geometry adds nothing to that.

The cost of replicating the geometry is memory: every rank holds a copy, so the memory a rank can spare limits the
size of the geometry, and a node with many ranks holds many identical copies. Two things reduce it, both with the
library's public calls:

* **Build once.** The geometry lives in a `Pool <doxygen/html/classEBGeometry_1_1Pool.html>`__
  (:ref:`Chap:MemoryModel`), which holds offsets rather than addresses, so its bytes are valid in any process. One
  rank builds the object (reads a mesh or a model, compiles a tape) and freezes its pool; the pool's size, its bytes
  and the object's ``Layout`` (offsets only) are broadcast. Every other rank receives the bytes into a pool made for
  them with ``Pool::forImage()`` and rebuilds the object from its layout and the pool's ``location()``, instead of
  reading and building it again (see :ref:`Sec:PoolImage`).
* **One copy per node.** A `MemoryResource <doxygen/html/classEBGeometry_1_1MemoryResource.html>`__ of your own,
  over an MPI shared-memory window (``MPI_Win_allocate_shared``), backs the pool on every rank of a node. Only one rank of the node receives the bytes,
  and every rank reads that copy, wherever the window is mapped in its process. A node then holds the geometry once,
  however many ranks run on it.

On a node with GPUs, each device still needs its own copy: freeze the pool and mirror it into device memory, as for
one process (:ref:`Sec:BuildingGPU`).

.. list-table::
   :header-rows: 1
   :widths: 22 78

   * - Folder
     - What it shows
   * - :file:`MPI/OctreeLevelSet`
     - Queries without communication: an adaptive octree whose root cells are split among the ranks in Morton order.
       Each rank reads the model into a ``Tape`` of its own, refines its roots toward the surface and the interfaces
       between regions, and stores the signed distance at each leaf's corners (and the region, ``Tape::region()``);
       one reduction gives a summary. Every rank writes its leaves as a VTK file, which ParaView and VisIt open
       through the ``.pvtu`` file rank 0 writes.
   * - :file:`MPI/SharedGeometry`
     - Build once, one copy per node: rank 0 reads a model and compiles it into a ``Tape``, and the other ranks
       rebuild the tape from its pool's bytes and its layout, never reading the file, with one copy per node in a
       shared-memory window behind a ``MemoryResource``. Every rank evaluates the same points through the shared
       copy and gets rank 0's values.

They read the model of :ref:`Chap:ExampleLanguageCSG` by default, and any other ``.ebg`` model as their first
argument; each checks its own answers and exits with 1 on an error. Each folder builds with GNU make (``mpicxx``) or CMake (``find_package(MPI)``) and runs with, for example,
``mpirun -n 4``; see the README in each folder.

.. _Sec:IntegrationsAMReX:

AMReX
-----

.. important::

   The AMReX integrations are run by hand, on CPUs and on an NVIDIA GPU (CUDA), against AMReX's development branch,
   and never with HIP.

Each AMReX integration mirrors one or more of the :ref:`Chap:Examples` and hands the object it builds to AMReX's
:cite:`AMReX2019` embedded-boundary grid generation as an implicit function, on a domain around the object's box with
cubic cells. The geometry lives in an EBGeometry ``Pool`` (see :ref:`Chap:MemoryModel`) that must outlive
every evaluation, and the implicit function holds the object by value. Several read a mesh from the ``common-3d-test-models``
git submodule by default (see :ref:`Sec:Cloning`).

Everything in EBGeometry is negative inside an object and positive outside it. AMReX reads
negative values as fluid, so the implicit functions of objects return the negated value, and the fluid is
around the object; a subdomain's function is handed over as it is, since the subdomain is where the code
computes. The sign is flipped at the boundary between the two codes, never by complementing the
EBGeometry object.

.. list-table::
   :header-rows: 1
   :widths: 18 57 25

   * - Folder
     - What it hands over
     - Mirrors
   * - :file:`Shapes`
     - One analytic shape, a torus (the least a coupling needs), or the classic CSG solid, compiled into a
       ``Tape`` or composed with the fused operations (``shape = torus``, ``csg`` or ``fused``).
     - :ref:`Chap:ExampleShapes`, :ref:`Chap:ExampleBasicCSG`, :ref:`Chap:ExampleFusedCSG`
   * - :file:`MeshSDF`
     - A surface mesh as a ``TriMeshSDF``.
     - :ref:`Chap:ExampleMeshSDF`
   * - :file:`MeshCSG`
     - The dragon on a table, blended by a smooth union, in a ``Tape``.
     - :ref:`Chap:ExampleMeshCSG`
   * - :file:`SurfaceLabels`
     - The armadillo with a bored cylinder, in a ``Tape``, and each cut cell labeled with its closest leaf
       (``closestLeaf()``) and, on the mesh, its closest face (``getClosestFace()``).
     - :ref:`Chap:ExampleSurfaceLabels`
   * - :file:`Materials`
     - One subdomain of a ``Domain`` (a cable's air, or its jacket), its own function cut, and each cut cell
       labeled with its surface group (the boundary condition's id), its role (a boundary or an interface) and
       the region on the other side (``Domain::closestSurface(p, subdomain)``). With ``eb.model`` (and
       ``eb.domain``), the object and the domain are read from a ``.ebg`` model instead.
     - :ref:`Chap:ExampleMaterials`, :ref:`Chap:ExampleLanguageDomains`
   * - :file:`ManyObjects`
     - 3,000 spheres in a cylinder joined by a ``BVHSmoothUnion`` (the fluid is between them), or a city of
       boxes in a ``BVHUnion`` (``scene = spheres`` or ``city``).
     - :ref:`Chap:ExamplePackedSpheres`, :ref:`Chap:ExampleManyObjects`

Building and running
~~~~~~~~~~~~~~~~~~~~

The :file:`Integrations/AMReX/<something>` folders couple EBGeometry to
`AMReX <https://amrex-codes.github.io/amrex/>`_'s ``EB2`` grid generation and write a plot file. AMReX must
be installed separately, with ``AMREX_HOME`` pointing to it.

The implicit function derives from ``amrex::GPUable``, so the same code runs on CPUs and, in a CUDA or HIP
build of AMReX, on GPUs. In a GPU build the pool is frozen and mirrored to device memory in one piece (see
:ref:`Sec:BuildingGPU`), and the implicit function holds two views of the object, one of each copy; each
compilation pass uses its own. An analytic shape is a plain value and needs no copy. A SYCL build of AMReX
is rejected at compile time, since EBGeometry has no SYCL memory resource.

A mesh's fine features can cut a cell edge more than twice, which ``EB2`` refuses unless it may cover such
cells, so the integrations set ``eb2.cover_multiple_cuts = 1`` unless the inputs say otherwise.

See the README in each folder for build and run instructions.
