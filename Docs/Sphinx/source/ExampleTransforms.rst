.. _Chap:ExampleTransforms:

Transforms
==========

One object moved, turned, scaled, mirrored, stretched and repeated. A transform applies to a whole
expression and changes where the object is, not what it is. The source is at :file:`Examples/Transforms/main.cpp`.

.. figure:: /_static/example_Transforms.jpg
   :width: 60%
   :align: center

   The object as built, rotated, scaled (back row); reflected, elongated, repeated (front row).

The transforms
--------------

The object is a ring around the :math:`y` axis with a post on its :math:`+x` side, so that every
transform shows:

* ``Translate(e, shift)`` moves it.
* ``Rotate(e, degrees, axis)`` turns it about a coordinate axis (0, 1, 2) or about any direction.
* ``Scale(e, factor)`` scales it uniformly about the origin, which keeps a distance a distance.
* ``Reflect(e, axis)`` mirrors it in a coordinate plane.
* ``Elongate(e, halfLengths)`` stretches it by inserting a box at the origin.
* ``FiniteRepetition(e, period, lo, hi)`` tiles copies over a period, ``lo`` and ``hi`` tiles to either
  side along each axis; the copies are evaluated as one sub-program.

The program prints each result's bounding box.

Looking at it
-------------

``Translate`` sets the six results out in two rows, compiled into one tape. A union of this many
operands is evaluated through a BVH over their boxes (see :ref:`Sec:BVHUnions`).

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit, scroll to zoom, X cuts the object open and P colours each primitive; see
:ref:`Chap:ImplemRender`.

Building and running
--------------------

See :ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler invocation.
It runs on the host only.

.. code-block:: bash

   cd Examples/Transforms
   ./Transforms.ex
