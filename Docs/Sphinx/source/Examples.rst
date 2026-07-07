.. _Chap:Examples:

Examples
========

The :file:`Examples/` folder contains pure ``EBGeometry`` examples with no third-party
dependencies.  Each one can be built directly with a compiler, with GNU Make, or with CMake
(see :ref:`Chap:BuildingDirectCompile`, :ref:`Chap:BuildingGNUMake`, :ref:`Chap:BuildingCMake`),
and every folder ships all three (a ``GNUmakefile``, a ``CMakeLists.txt``, and a plain
``main.cpp`` compilable with a single compiler invocation).  These are covered below.

Examples that couple ``EBGeometry`` to a third-party application code's embedded-boundary grid
generation (`AMReX <https://amrex-codes.github.io/amrex/>`_, `Chombo
<https://commons.lbl.gov/display/chombo/>`_) live separately under :file:`ThirdParty/` instead,
and additionally require that platform to be installed — see :ref:`Chap:AMReXExamples`.

None of the ``EBGeometry`` examples below produce output for visualization; they print timing
and/or correctness information to the terminal.  Each example folder also ships its own
``README.md`` with the exact build/run commands reproduced here.

.. contents:: On this page
   :local:
   :depth: 1

Building and running any of them
----------------------------------

Every example needs the path to the ``EBGeometry`` root — the
directory containing ``EBGeometry.hpp`` — which is two levels up (``../..``) when building in
place. All three build methods accept the same two overrides: ``PRECISION``/``EBGEOMETRY_PRECISION``
(``float`` or ``double``, default ``double``) and the location of the ``EBGeometry`` tree
itself (``EBGEOMETRY_HOME``, default ``../..``); all three also produce the binary
(``<Example>.ex``) directly in the example's own folder, regardless of where the CMake build
tree itself lives:

.. code-block:: bash

   # CMake
   cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry
   cmake --build build
   ./<Example>.ex

   # GNU Make
   make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry
   ./<Example>.ex

   # Direct compilation
   g++ -std=c++17 -O3 -march=native -DEBGEOMETRY_PRECISION=float -I../.. main.cpp -o <Example>.ex
   ./<Example>.ex

Run every example from inside its own folder — several resolve a default input mesh relative to
the run directory.

Shapes
------

A very basic example of using ``EBGeometry`` for creating analytic signed distance fields
(see :ref:`Chap:ImplemCSG`).  A good starting point for exploring the library's built-in shapes
and transformations.

.. code-block:: bash

   cd Examples/Shapes
   ./Shapes.ex

MeshSDF
-------

Reads a surface mesh and evaluates its signed distance field using all three mesh-SDF
representations described in :ref:`Chap:MeshSDFClasses`:

* A naive :math:`O(N)` scan over all facets (``FlatMeshSDF``).
* A ``PackedBVH`` with pointer-free, index-offset nodes storing references to the facets directly (``MeshSDF``).
* A SIMD-accelerated, SoA-packed triangle BVH (``TriMeshSDF``).

.. tip::

   SDF query complexity depends on both the geometry and the query point. A tessellated sphere
   has a "blind spot" at its center where even a BVH must visit most, if not all, primitives —
   this example is a good way to see that effect in practice.

.. code-block:: bash

   cd Examples/MeshSDF
   ./MeshSDF.ex                                            # defaults to armadillo.obj
   ./MeshSDF.ex ../../common-3d-test-models/data/cow.obj   # or pick another mesh

With no argument the example loads ``armadillo.obj`` from the ``common-3d-test-models``
submodule, so make sure it is checked out first (see :ref:`Sec:Cloning`).

CSGUnion
--------

Builds a CSG *union* of two different kinds of implicit function — a signed distance field read
from a surface mesh, and an analytic sphere.  Both derive from ``ImplicitFunction<T>``, so they
combine directly through ``BVHUnion``, which places the objects in a bounding volume hierarchy so
that closest-object queries traverse the tree instead of testing every object (see
:ref:`Chap:BVH` and :ref:`Chap:ImplemCSG`).

.. code-block:: bash

   cd Examples/CSGUnion
   ./CSGUnion.ex                                           # defaults to cow.obj
   ./CSGUnion.ex ../../common-3d-test-models/data/cow.obj

PackedSpheres
-------------

Creates a scene of :math:`N^3` analytic spheres and combines them with two kinds of union: a
standard union that scans every object, and a BVH-accelerated union
(see :ref:`Chap:BVH`).  Demonstrates the algorithmic-complexity benefit of the BVH for
closest-object queries as the object count grows.

.. code-block:: bash

   cd Examples/PackedSpheres
   ./PackedSpheres.ex

RandomCity
----------

Creates a scene of randomly placed boxes ("buildings") and, like ``PackedSpheres``,
compares a standard union against a BVH-accelerated union for closest-object queries.

.. code-block:: bash

   cd Examples/RandomCity
   ./RandomCity.ex

OctreeBoundingVolume
---------------------

Computes an approximate bounding volume for an implicit function using octree refinement (see
:ref:`Chap:ImplemOctree` and ``ImplicitFunction::approximateBoundingVolumeOctree``).

.. code-block:: bash

   cd Examples/OctreeBoundingVolume
   ./OctreeBoundingVolume.ex
