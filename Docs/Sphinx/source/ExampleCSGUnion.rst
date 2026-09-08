.. _Chap:ExampleCSGUnion:

CSGUnion
=========

.. warning::

   **Temporarily disabled.** The BVH-accelerated CSG unions (``BVHUnionIF``, ``BVHSmoothUnionIF``
   and their ``BVHUnion``/``BVHSmoothUnion`` factories) are compiled out during the GPU port, behind
   ``EBGEOMETRY_ENABLE_BVH_CSG_UNION`` in :file:`Source/EBGeometry_CSG.hpp`. They stored their
   primitives as ``std::shared_ptr<const ImplicitFunction<T>>``, which the removed
   ``BVH::SharedPtrStorage`` policy provided and neither remaining policy can (see
   :ref:`Sec:PolymorphicPrimitives`). They return with the index-based redesign of the
   implicit-function and CSG layer. Until then this program builds, but prints a notice and exits
   without doing any work.

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
