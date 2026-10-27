.. _Chap:Integrations:

Integrations
============

.. important::

   These integrations show how EBGeometry is coupled to a third-party application code. They are not
   built as part of EBGeometry's continuous integration: unlike everything under :ref:`Chap:Examples`,
   which is compiled and run on every pull request, a change to a third-party platform (or to EBGeometry
   itself) can break them without anyone noticing. All of them were last compiled and run in October 2026,
   on CPUs, against AMReX's development branch (26.09) and Chombo 3.3. Treat them as a starting point for
   your own integration.

Each integration mirrors one of the :ref:`Chap:Examples` and hands the object it builds to the platform's
embedded-boundary grid generation as an implicit function, on a cubic domain around the object's box. The
geometry lives in an EBGeometry ``Pool`` (see :ref:`Chap:MemoryModel`) that must outlive every evaluation,
and the implicit function holds the object by value. Several read a mesh from the ``common-3d-test-models``
git submodule by default (see :ref:`Sec:Cloning`).

Everything in EBGeometry is negative inside an object and positive outside it. AMReX and Chombo read
negative values as fluid, so every implicit function here returns the negated value, and the fluid is
around the object. The sign is flipped at the boundary between the two codes, never by complementing the
EBGeometry object.

.. list-table::
   :header-rows: 1
   :widths: 18 52 15 15

   * - Folder
     - What it hands over
     - Mirrors
     - Platforms
   * - :file:`Shapes`
     - One analytic shape, a torus: the least a coupling needs.
     - :ref:`Chap:ExampleShapes`
     - AMReX, Chombo
   * - :file:`BasicCSG`
     - The classic CSG solid, compiled into a ``Tape``.
     - :ref:`Chap:ExampleBasicCSG`
     - AMReX, Chombo
   * - :file:`MeshSDF`
     - A surface mesh as a ``TriMeshSDF``.
     - :ref:`Chap:ExampleMeshSDF`
     - AMReX, Chombo
   * - :file:`MeshCSG`
     - The dragon on a table, blended by a smooth union, in a ``Tape``.
     - :ref:`Chap:ExampleMeshCSG`
     - AMReX, Chombo
   * - :file:`PackedSpheres`
     - 3,000 spheres in a cylinder, joined by a ``BVHSmoothUnion``: the fluid is between them.
     - :ref:`Chap:ExamplePackedSpheres`
     - AMReX, Chombo
   * - :file:`RandomCity`
     - A city of boxes in a ``BVHUnion``.
     - :ref:`Chap:ExampleManyObjects`
     - AMReX, Chombo
   * - :file:`PaintEB`
     - A mesh as a ``MeshSDF``, and each cut cell labelled with the id of the closest mesh face.
     - :ref:`Chap:ExampleMeshSDF`
     - AMReX

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

Chombo
-------

The :file:`Integrations/Chombo/<something>` folders couple EBGeometry to
`Chombo <https://commons.lbl.gov/display/chombo/>`_'s ``GeometryShop``, on CPUs, and write the implicit
function's value in every cell to an HDF5 file. Chombo must be installed separately, built as C++17
(``CXXSTD = 17``; Chombo compiles as C++11 by default), with ``CHOMBO_HOME`` pointing to its :file:`lib`
folder. The implicit function derives from ``BaseIF``; Chombo copies it, and every copy holds the same
handle into the one pool.

See the README in each folder for build and run instructions.
