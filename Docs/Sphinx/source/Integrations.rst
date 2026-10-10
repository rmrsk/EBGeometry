.. _Chap:Integrations:

Integrations
============

.. important::

   These examples show how EBGeometry can be integrated into a third-party application code. They
   are not built as part of EBGeometry's continuous integration: unlike everything under
   :ref:`Chap:Examples`, which is compiled and run on every pull request, a change to a
   third-party platform (or to EBGeometry itself) can break them without anyone noticing. The
   ones marked *works* below were last compiled and run against AMReX's development branch
   (September 2026) and Chombo 3.2, on CPUs. Treat them as a starting point for your own
   integration.

Each integration hands an EBGeometry signed distance function to the platform's
embedded-boundary grid generation, which uses it to cut cells at the implicit surface. The
geometry lives in an EBGeometry ``Pool`` (see :ref:`Chap:MemoryModel`) that must outlive every
evaluation, and the implicit function holds the geometry's descriptors by value.

AMReX
------

The :file:`Integrations/AMReX/<something>` folders couple EBGeometry to
`AMReX <https://amrex-codes.github.io/amrex/>`_'s ``EB2`` grid generation. AMReX must be
installed separately, with the ``AMREX_HOME`` environment variable pointing to it.

The implicit functions derive from ``amrex::GPUable``, so the same code runs on CPUs and, in a
CUDA or HIP build of AMReX, on GPUs: in a GPU build the pool is frozen and mirrored to the device
in one piece, and the implicit function holds a host descriptor and a device descriptor of the
same geometry (see :ref:`Sec:BuildingGPU`). A SYCL build of AMReX is rejected at compile time,
since EBGeometry has no SYCL memory resource yet.

.. list-table::
   :header-rows: 1
   :widths: 20 55 25

   * - Folder
     - What it shows
     - Status
   * - :file:`MeshSDF`
     - A surface mesh as a ``TriMeshSDF``.
     - Works
   * - :file:`PaintEB`
     - A surface mesh as a ``MeshSDF``; each cut cell is painted with the id of the mesh face
       closest to it, from ``getClosestFace()``.
     - Works
   * - :file:`PackedSpheres`
     - 8000 spheres blended by a ``BVHSmoothUnion``; ``bvh = false`` visits every sphere instead,
       for comparison.
     - Works
   * - :file:`RandomCity`
     - 2500 boxes joined by a ``BVHUnion``; ``bvh = false`` as above.
     - Works
   * - :file:`Shapes`
     - The analytic shapes, some of them transformed or combined.
     - Parked until the tape

Chombo
-------

The :file:`Integrations/Chombo/<something>` folders couple EBGeometry to
`Chombo <https://commons.lbl.gov/display/chombo/>`_'s embedded-boundary grid generation, in the
same spirit as the AMReX examples above, on CPUs. Chombo must be installed separately, with the
``CHOMBO_HOME`` environment variable pointing to it, and the examples built with ``CXXSTD=17``,
since Chombo compiles as C++11 by default. Chombo copies implicit functions, so each holds its
geometry by value and the pool by ``shared_ptr``: every copy shares the one pool.

.. list-table::
   :header-rows: 1
   :widths: 20 55 25

   * - Folder
     - What it shows
     - Status
   * - :file:`MeshSDF`
     - A surface mesh as a ``TriMeshSDF``.
     - Works
   * - :file:`PackedSpheres`
     - 8000 spheres blended by a ``BVHSmoothUnion``.
     - Works
   * - :file:`RandomCity`
     - 100 boxes joined by a ``BVHUnion``.
     - Works
   * - :file:`Shapes`
     - The analytic shapes, some of them transformed.
     - Parked until the tape

See the README in each folder for exact build and run instructions.

The two Shapes integrations
---------------------------

The Shapes integrations compose the analytic shapes through the virtual transform and CSG layer
(``Offset``, ``Annular``, ``Complement``, ...), which the shapes no longer derive from now that they
are plain value types that can run on a device (see :ref:`Sec:AnalyticShapes`). They do not compile
against the current API. Composing value-type shapes returns with the tape, EBGeometry's
value-type expression builder; the two are kept as its acceptance tests, and will be ported to it.
