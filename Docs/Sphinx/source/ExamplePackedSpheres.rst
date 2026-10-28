.. _Chap:ExamplePackedSpheres:

Packed spheres
==============

Spheres of random sizes filling a cylinder, joined by a smooth union so that neighbours that touch merge
with a fillet. The source is at :file:`Examples/PackedSpheres/main.cpp`.

.. figure:: /_static/example_PackedSpheres.jpg
   :width: 60%
   :align: center

   3,000 spheres in a cylinder, joined by a smooth union.

Building the packing
--------------------

Each sphere's centre is drawn by rejection sampling: uniformly in the box around a cylinder along
:math:`x`, five times as long as its radius, and kept only if it lies inside the cylinder. Its radius is
drawn too. A
`BVHSmoothUnion <doxygen/html/classEBGeometry_1_1BVHSmoothUnion.html>`__ holds them in a pool and builds a
BVH over their boxes:

.. code-block:: c++

   const BVHSmoothUnion<T, SphereSDF<T>, K, SmoothMinOp<T>> packing(pool, spheres, boxes, blendLength);

A point is evaluated against the spheres near it only, and the two nearest values are blended with the
smooth minimum, so spheres within the blend length of each other merge (see :ref:`Sec:BVHUnions`). The
program prints the number of spheres, the box and the value at two points.

Looking at it
-------------

The example ends with ``Render::view(packing, argc, argv)``; X cuts the cylinder open to show the
spheres inside.

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit, scroll to zoom, X cuts the object open, P cycles the colouring and H hides the overlay; see
:ref:`Chap:ImplemRender`.

Building and running
--------------------

See :ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler invocation.
Built for a GPU (see :ref:`Sec:ExamplesGPU`), the window renders on the device.

.. code-block:: bash

   cd Examples/PackedSpheres
   ./PackedSpheres.ex
