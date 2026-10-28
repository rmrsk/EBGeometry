.. _Chap:ExampleFusedCSG:

Fused CSG
=========

The solid of :ref:`Chap:ExampleBasicCSG` built as a fused composition (see :ref:`Sec:Fused`): a shape
whose type spells out the whole tree, evaluated with nothing to interpret. The source is at
:file:`Examples/FusedCSG/main.cpp`.

.. figure:: /_static/example_FusedCSG.jpg
   :width: 60%
   :align: center

   The scene of two fused parts.

Building it fused
-----------------

The functions in namespace ``Fused`` (`EBGeometry_Fused.hpp
<doxygen/html/EBGeometry__Fused_8hpp.html>`__) are the builder's operations as types. They are called
qualified (``Fused::Union``, not ``Union``, which is the builder's), and where the builder takes a
``std::vector`` they take the operands as arguments:

.. code-block:: c++

   const auto fused = Fused::Difference(Fused::Intersection(cube, ball), drillX, drillY, drillZ);

Against the tape
----------------

The program builds the same solid through the builder and a ``Tape`` and prints both values at a point,
which are the same to rounding, and the time per point of each on the host: the tape fetches and
dispatches each clause, the fused solid has nothing to interpret.

Fused parts in a tape
---------------------

A fused solid is one shape with one surface. Listed after ``T`` it is one leaf of a tape, so a scene
known only at run time can place compiled parts, and ``closestPrimitive()`` names the part:

.. code-block:: c++

   const Tape<T, FusedSolid> scene(pool, Union(Expr<T>(fused), Translate(Rotate(fused, 45, 1), Vec3(3, 0, 0))));

The same query on a GPU
-----------------------

The fused solid holds its shapes, not offsets into a pool, so it goes to the device as it is:
``GPU::transform(Distance<FusedSolid>{fused}, line, values)``, with nothing to mirror.

Looking at it
-------------

The example ends with ``Render::view(scene, pool, argc, argv)``, a window on the scene of two fused parts.

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit and scroll to zoom; the window lists its other keys (see
:ref:`Sec:ViewerKeys`).

Building and running
--------------------

See :ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler invocation,
and :ref:`Sec:ExamplesGPU` for a GPU build.

.. code-block:: bash

   cd Examples/FusedCSG
   ./FusedCSG.ex
