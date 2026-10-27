.. _Chap:ExampleValueOperations:

Value operations
================

``Offset``, ``Annular``, ``Blur`` and ``Mollify`` change an object's value rather than where it is. The
source is at :file:`Examples/ValueOperations/main.cpp`.

.. figure:: /_static/example_ValueOperations.jpg
   :width: 60%
   :align: center

   The cube, offset outward and inward (back row); the open shell, blurred, mollified (front row).

The operations
--------------

Each is applied to a cube:

* ``Offset(e, r)`` moves the surface outward by :math:`r`, inward if :math:`r` is negative. Growing an
  exact distance keeps it exact, which rounds the cube's edges; shrinking it gives a bound.
* ``Annular(e, d)`` keeps a shell of half-thickness :math:`d` around the surface. The program cuts the
  shell's top off with a ``Difference`` so that its wall shows.
* ``Blur(e, d)`` averages the value over 27 points spaced :math:`d` apart, and ``Mollify(e, r, n)`` over
  an :math:`n \times n \times n` grid of half-width :math:`r` with a smooth weight, at :math:`n^3`
  evaluations per point. Both round edges and corners; their surfaces are averages of a few samples,
  which the shading shows as faint facets.

The program prints each object's value at the cube's corner and whether it is still an exact distance.

Looking at it
-------------

The viewer shows the six objects in two rows: the cube, grown and shrunk, then the open shell, blurred
and mollified.

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit, scroll to zoom, X cuts the object open and P colours each primitive; see
:ref:`Chap:ImplemRender`.

Building and running
--------------------

See :ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler invocation.
It runs on the host only.

.. code-block:: bash

   cd Examples/ValueOperations
   ./ValueOperations.ex
