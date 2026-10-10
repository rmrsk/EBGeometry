.. _Chap:ExamplePackedSpheres:

PackedSpheres
==============

Creates a scene of :math:`N^3` analytic spheres and combines them with two kinds of union: a
standard union that scans every object, and a BVH-accelerated union
(see :ref:`Chap:BVH`).  Demonstrates the algorithmic-complexity benefit of the BVH for
closest-object queries as the object count grows. The two must give the same distance at every
sample point, up to rounding, since the BVH only skips spheres that cannot be the nearest.

The source for this example is at :file:`Examples/PackedSpheres/main.cpp`. See
:ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler
invocation.

.. code-block:: bash

   cd Examples/PackedSpheres
   ./PackedSpheres.ex
