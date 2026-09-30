.. _Chap:ExampleCSGUnion:

CSGUnion
=========

.. warning::

   **Temporarily disabled.** This example unions two objects of *different* types, a mesh distance
   field and an analytic sphere. Neither is an ``ImplicitFunction<T>`` any more (see
   :ref:`Sec:AnalyticShapes` and :ref:`Chap:MeshSDFClasses`), and ``BVHUnionIF`` holds primitives of
   a single type (see :ref:`Sec:BVHUnions`). A union of different types needs the runtime dispatch
   of the tape, which is still to come. Until then this program builds, but prints a notice and
   exits without doing any work.

Builds a CSG *union* of two different kinds of signed distance field — one read from a surface
mesh, and an analytic sphere. The program was written when both derived from
``ImplicitFunction<T>`` and so combined directly through ``BVHUnion``, which places the objects in
a bounding volume hierarchy so that closest-object queries traverse the tree instead of testing every object (see
:ref:`Chap:BVH` and :ref:`Chap:ImplemCSG`).

The source for this example is at :file:`Examples/CSGUnion/main.cpp`. See :ref:`Chap:Building`
for how to compile it with CMake, GNU Make, or a direct compiler invocation.

.. code-block:: bash

   cd Examples/CSGUnion
   ./CSGUnion.ex                                           # defaults to cow.obj
   ./CSGUnion.ex ../../common-3d-test-models/data/cow.obj
