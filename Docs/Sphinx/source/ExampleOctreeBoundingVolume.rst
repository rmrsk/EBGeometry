.. _Chap:ExampleOctreeBoundingVolume:

OctreeBoundingVolume
======================

Computes approximate bounding volumes using octree refinement (see :ref:`Sec:OctreeBoundingVolume`
and the free function ``approximateBoundingVolumeOctree``): first for a single analytic shape (a
cone) passed in directly, then for a lambda that takes the union of three shapes.

The source for this example is at :file:`Examples/OctreeBoundingVolume/main.cpp`. See
:ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler
invocation.

.. code-block:: bash

   cd Examples/OctreeBoundingVolume
   ./OctreeBoundingVolume.ex
