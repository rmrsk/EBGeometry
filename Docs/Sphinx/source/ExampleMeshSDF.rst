.. _Chap:ExampleMeshSDF:

MeshSDF
========

Reads a surface mesh and evaluates its signed distance field using all three mesh-SDF
representations described in :ref:`Chap:MeshSDFClasses`:

* A naive :math:`O(N)` scan over all facets (``FlatMeshSDF``).
* A ``PackedBVH`` with pointer-free, index-offset nodes storing face ids, each test reading its face from the mesh (``MeshSDF``).
* A SIMD-accelerated, SoA-packed triangle BVH (``TriMeshSDF``).

It times each at the same random points around the mesh, and checks that the three agree at every
point, to a tolerance scaled by the mesh's size. On a mesh with holes they can disagree on the sign
near a hole, where the inside is not well defined.

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
