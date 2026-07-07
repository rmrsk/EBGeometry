.. _Chap:AMReXExamples:

AMReX examples
================

The :file:`ThirdParty/AMReX/<something>` folders couple ``EBGeometry`` to
`AMReX <https://amrex-codes.github.io/amrex/>`_'s embedded-boundary (EB) grid generation: an
``EBGeometry`` signed distance function is handed to AMReX, which uses it to cut cells at the
implicit surface.  They are intended to expose the same underlying ``EBGeometry`` features as
the pure examples in :ref:`Chap:Examples`, but driven through AMReX's grid-generation and MPI
infrastructure instead of a standalone ``main()``.

.. important::

   Unlike the examples in :ref:`Chap:Examples`, everything under :file:`ThirdParty/` is
   illustrative only and not built or run as part of continuous integration -- see
   :file:`ThirdParty/README.md` at the repository root for why.

Requirements
------------

These require `AMReX
<https://amrex-codes.github.io/amrex/>`_ to be installed separately, with the ``AMREX_HOME``
environment variable pointing to it.  Compile each example with AMReX's standard GNU Make
workflow from inside its folder:

.. code-block:: bash

   cd ThirdParty/AMReX/<something>
   make -s -j8 DIM=3

and run the resulting executable (named ``main3d.<config>.ex``, where ``<config>`` encodes the
compiler/MPI/dimension configuration AMReX was built with) with or without MPI:

.. code-block:: bash

   mpirun -np 8 main3d.<config>.ex eb2.cover_multiple_cuts=1 which_geom=0

Some geometries generate cut-cells with multiple disjoint fragments per cell, which AMReX does
not natively support — running with ``eb2.cover_multiple_cuts=1`` merges those fragments instead
of erroring out.

Common input options
-----------------------

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Option
     - Effect
   * - ``n_cell = <integer>``
     - Number of grid cells along each coordinate direction.
   * - ``max_grid_size = <integer>``
     - AMReX blocking factor (box size used for domain decomposition).
   * - ``num_coarsen_opt = <integer>``
     - Performance tuning for the EB generation (number of coarsening optimization passes).
   * - ``which_geom = <integer>``
     - Selects the geometry, where supported — see each example below.
   * - ``use_bvh = true/false`` or ``bvh = true/false``
     - Turns the BVH accelerator on/off for the CSG/closest-object query, where supported (the
       exact option name varies per example — see below).

.. contents:: On this page
   :local:
   :depth: 1

Shapes
-------------

Uses AMReX's EB grid generation directly on ``EBGeometry``'s analytic signed distance fields
(see :ref:`Chap:ImplemCSG`).  Selectable via ``which_geom``:

.. list-table::
   :widths: 20 80
   :header-rows: 1

   * - ``which_geom``
     - Geometry
   * - 0
     - Sphere
   * - 1
     - Plane
   * - 2
     - Infinite cylinder
   * - 3
     - Finite cylinder
   * - 4
     - Capsule
   * - 5
     - Box
   * - 6, 12
     - Rounded box
   * - 7
     - Torus
   * - 8
     - Infinite cone
   * - 9
     - Finite cone
   * - 10
     - Spherical shell
   * - 11
     - Death star
   * - 13
     - Perlin gradient-noise SDF

MeshSDF
--------------

Uses AMReX's EB grid generation on a surface-mesh signed distance field (see
:ref:`Chap:MeshSDFClasses`), with ``use_bvh = true/false`` toggling the BVH accelerator.
Selectable via ``which_geom``:

.. list-table::
   :widths: 20 80
   :header-rows: 1

   * - ``which_geom``
     - Geometry
   * - 0
     - Airfoil
   * - 1
     - Sphere
   * - 2
     - Dodecahedron
   * - 3
     - Horse
   * - 4
     - Porsche
   * - 5
     - Orion capsule
   * - 6
     - Armadillo
   * - 7
     - Adirondack

PackedSpheres
---------------------

The AMReX-coupled counterpart of ``PackedSpheres`` (see :ref:`Chap:Examples`): constructs a
packed spherical bed and cuts AMReX cells against it, with ``bvh = true/false`` toggling the
BVH-accelerated CSG union.

RandomCity
------------------

The AMReX-coupled counterpart of ``RandomCity`` (see :ref:`Chap:Examples`): constructs a random
urban city environment (boxes) and cuts AMReX cells against it, with ``bvh = true/false``
toggling the BVH-accelerated CSG union.

PaintEB
---------------

Associates triangle metadata (the ``Meta`` template parameter described in :ref:`Chap:DCEL`)
with the AMReX-generated cut-cells, so that per-triangle data (e.g. a material ID or boundary
condition tag) can be carried through into the discrete EB representation.  Uses the same
``which_geom`` selection as ``MeshSDF`` above.
