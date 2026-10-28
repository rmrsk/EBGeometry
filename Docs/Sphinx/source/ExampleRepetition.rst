.. _Chap:ExampleRepetition:

Repetition
==========

Many copies of one object on a grid with ``FiniteRepetition``; here a mesh, the happy Buddha. The source
is at :file:`Examples/Repetition/main.cpp`.

.. figure:: /_static/example_Repetition.jpg
   :width: 60%
   :align: center

   49 copies of the happy Buddha from one ``FiniteRepetition``.

.. important::

   The default mesh comes from the ``common-3d-test-models`` git submodule. Clone with
   ``--recurse-submodules``, or run ``git submodule update --init`` in the repository (see
   :ref:`Sec:Cloning`); otherwise pass a mesh of your own as the first argument.

Repeating a mesh
----------------

The mesh is read into a ``TriMeshSDF``, scaled to height 1 and stood on :math:`y = 0` with ``Scale`` and
``Translate``. ``FiniteRepetition(object, period, lo, hi)`` tiles it with the given period, ``lo`` and
``hi`` tiles to either side of the home tile along each axis:

.. code-block:: c++

   const Expr grid = FiniteRepetition(statue, Vec3(0.8, 1, 0.8), Vec3(3, 0, 3), Vec3(3, 0, 3));   // 7 x 7

The tape stores the object once and evaluates it as a sub-program: each point is folded into the tile it
lies in, and only the copies whose boxes come closer than that copy's value are evaluated after it.

Against a union of copies
-------------------------

The program also places the same 49 copies one by one with ``Translate`` in a ``Union``, which the tape
evaluates through a BVH over their boxes (see :ref:`Sec:BVHUnions`). Both give the same values; the
program prints each tape's number of clauses, which grows with the copies only for the union, and the
time per point of each.

Looking at it
-------------

The example ends with ``Render::view(repeated, pool, argc, argv)``, a window on the grid of statues.

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit and scroll to zoom; the window lists its other keys (see
:ref:`Sec:ViewerKeys`).

Building and running
--------------------

See :ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler invocation.
Built for a GPU (see :ref:`Sec:ExamplesGPU`), the window renders on the device.

.. code-block:: bash

   cd Examples/Repetition
   ./Repetition.ex
