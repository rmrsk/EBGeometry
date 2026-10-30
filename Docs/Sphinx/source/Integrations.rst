.. _Chap:Integrations:

Integrations
============

.. important::

   These integrations show how EBGeometry is coupled to a third-party application code. They are not
   built as part of EBGeometry's continuous integration: unlike everything under :ref:`Chap:Examples`,
   which is compiled and run on every pull request, a change to a third-party platform (or to EBGeometry
   itself) can break them without anyone noticing. All of them were last compiled and run in October 2026,
   on CPUs and on an NVIDIA GPU (CUDA), against AMReX's development branch (26.09); they have not been run
   with HIP. Treat them as a starting point for your own integration.

Each integration mirrors one or more of the :ref:`Chap:Examples` and hands the object it builds to AMReX's
embedded-boundary grid generation as an implicit function, on a domain around the object's box with
cubic cells. The geometry lives in an EBGeometry ``Pool`` (see :ref:`Chap:MemoryModel`) that must outlive
every evaluation, and the implicit function holds the object by value. Several read a mesh from the ``common-3d-test-models``
git submodule by default (see :ref:`Sec:Cloning`).

Everything in EBGeometry is negative inside an object and positive outside it. AMReX reads
negative values as fluid, so every implicit function here returns the negated value, and the fluid is
around the object. The sign is flipped at the boundary between the two codes, never by complementing the
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
   * - :file:`Materials`
     - The armadillo with a bored cylinder, in a ``Tape``, and each cut cell labelled with its closest leaf
       (``closestPrimitive()``) and, on the mesh, its closest face (``getClosestFace()``).
     - :ref:`Chap:ExampleMaterials`
   * - :file:`ManyObjects`
     - 3,000 spheres in a cylinder joined by a ``BVHSmoothUnion`` (the fluid is between them), or a city of
       boxes in a ``BVHUnion`` (``scene = spheres`` or ``city``).
     - :ref:`Chap:ExamplePackedSpheres`, :ref:`Chap:ExampleManyObjects`

AMReX
------

The :file:`Integrations/AMReX/<something>` folders couple EBGeometry to
`AMReX <https://amrex-codes.github.io/amrex/>`_'s ``EB2`` grid generation and write a plot file. AMReX must
be installed separately, with ``AMREX_HOME`` pointing to it.

The implicit function derives from ``amrex::GPUable``, so the same code runs on CPUs and, in a CUDA or HIP
build of AMReX, on GPUs. In a GPU build the pool is frozen and mirrored to device memory in one piece (see
:ref:`Sec:BuildingGPU`), and the implicit function holds two views of the object, one of each copy; each
compilation pass uses its own. An analytic shape is a plain value and needs no copy. A SYCL build of AMReX
is rejected at compile time, since EBGeometry has no SYCL memory resource yet.

A mesh's fine features can cut a cell edge more than twice, which ``EB2`` refuses unless it may cover such
cells, so the integrations set ``eb2.cover_multiple_cuts = 1`` unless the inputs say otherwise.

See the README in each folder for build and run instructions.
