.. _Chap:ExampleSmoothCSG:

Smooth CSG
==========

``SmoothUnion``, ``SmoothIntersection`` and ``SmoothDifference`` blend two objects over a length instead
of meeting at a sharp crease (see :ref:`Chap:ConstructiveSolidGeometry`). The source is at :file:`Examples/SmoothCSG/main.cpp`.

.. figure:: /_static/example_SmoothCSG.jpg
   :width: 60%
   :align: center

   Sharp operations at the back, smooth ones in front: union, intersection, difference.

Blending
--------

Each smooth function takes the blend length after its operands, and optionally a
`BlendKind <doxygen/html/EBGeometry__Expr_8hpp.html>`__:

.. code-block:: c++

   const Expr smooth      = SmoothUnion(left, right, blend);   // BlendKind::Polynomial
   const Expr exponential = SmoothUnion(left, right, blend, BlendKind::Exponential);

The polynomial blend changes the value only within a band of the blend length around the crease; the
exponential blend changes it everywhere, most near the crease, and rounds the result a little more.
Where the two spheres' surfaces meet the sharp union is zero, and the program prints how far below
zero each smooth union is there: the material the blend adds.

Looking at it
-------------

The viewer shows the sharp union, intersection and difference in a row at the back, and the smooth
ones in front, all in one tape.

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit, scroll to zoom, X cuts the object open and P colours each primitive; see
:ref:`Chap:ImplemRender`.

Building and running
--------------------

See :ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler invocation.
It runs on the host only.

.. code-block:: bash

   cd Examples/SmoothCSG
   ./SmoothCSG.ex
