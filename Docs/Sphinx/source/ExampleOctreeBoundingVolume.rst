.. _Chap:ExampleOctreeBoundingVolume:

OctreeBoundingVolume
======================

Computes approximate bounding volumes using octree refinement (see :ref:`Sec:OctreeBoundingVolume`
and the free function ``approximateBoundingVolumeOctree``): first for a single analytic shape (a
cone) passed in directly, then for a lambda that takes the union of three shapes. It checks both
estimates against the exact boxes the shapes give from ``computeBoundingVolume()``: each must hold
the exact box and overshoot it by no more than one leaf cell.

Compiled for a GPU, the same run also evaluates both bounding-box estimates in a kernel, one octree
depth per thread, and compares the results with the host's; see :ref:`Sec:ExamplesGPU`.

The source for this example is at :file:`Examples/OctreeBoundingVolume/main.cpp`. See
:ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler
invocation.

.. code-block:: bash

   cd Examples/OctreeBoundingVolume
   ./OctreeBoundingVolume.ex
