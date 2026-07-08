.. _Chap:ExampleCSGUnion:

CSGUnion
=========

Builds a CSG *union* of two different kinds of implicit function — a signed distance field read
from a surface mesh, and an analytic sphere.  Both derive from ``ImplicitFunction<T>``, so they
combine directly through ``BVHUnion``, which places the objects in a bounding volume hierarchy so
that closest-object queries traverse the tree instead of testing every object (see
:ref:`Chap:BVH` and :ref:`Chap:ImplemCSG`).

The source for this example is at :file:`Examples/CSGUnion/main.cpp`. See :ref:`Chap:Building`
for how to compile it with CMake, GNU Make, or a direct compiler invocation.

.. code-block:: bash

   cd Examples/CSGUnion
   ./CSGUnion.ex                                           # defaults to cow.obj
   ./CSGUnion.ex ../../common-3d-test-models/data/cow.obj
