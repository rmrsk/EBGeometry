.. _Chap:ExampleMeshSDF:

MeshSDF
========

Reads a surface mesh and evaluates its signed distance field using all three mesh-SDF
representations described in :ref:`Chap:MeshSDFClasses`:

* A naive :math:`O(N)` scan over all facets (``FlatMeshSDF``).
* A ``PackedBVH`` with pointer-free, index-offset nodes storing references to the facets directly (``MeshSDF``).
* A SIMD-accelerated, SoA-packed triangle BVH (``TriMeshSDF``).

.. tip::

   SDF query complexity depends on both the geometry and the query point. A tessellated sphere
   has a "blind spot" at its center where even a BVH must visit most, if not all, primitives —
   this example is a good way to see that effect in practice.

The source for this example is at :file:`Examples/MeshSDF/main.cpp`. See :ref:`Chap:Building`
for how to compile it with CMake, GNU Make, or a direct compiler invocation.

.. code-block:: bash

   cd Examples/MeshSDF
   ./MeshSDF.ex                                            # defaults to armadillo.obj
   ./MeshSDF.ex ../../common-3d-test-models/data/cow.obj   # or pick another mesh

With no argument the example loads ``armadillo.obj`` from the ``common-3d-test-models``
submodule, so make sure it is checked out first (see :ref:`Sec:Cloning`).
