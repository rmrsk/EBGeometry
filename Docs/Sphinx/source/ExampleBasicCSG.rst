.. _Chap:ExampleBasicCSG:

Basic CSG
=========

The classic constructive solid geometry solid (see :ref:`Chap:ConstructiveSolidGeometry`): a cube with
its corners rounded off by a ball, drilled through along all three axes. It is described with the
expression builder, compiled into a ``Tape`` (see :ref:`Sec:Tape`), and evaluated on the host and on a
GPU. The source is at :file:`Examples/BasicCSG/main.cpp`.

.. figure:: /_static/example_BasicCSG.jpg
   :width: 60%
   :align: center

   The solid as the viewer first shows it.

Describing the solid
--------------------

Every analytic shape converts to an `Expr <doxygen/html/classEBGeometry_1_1Expr.html>`__, and the
builder functions in `EBGeometry_Expr.hpp <doxygen/html/EBGeometry__Expr_8hpp.html>`__ combine
expressions without evaluating anything. ``Intersection`` keeps what is inside both operands and
``Difference`` removes the second from the first; like ``Union``, both take a pair or a ``std::vector``:

.. code-block:: c++

   const Expr solid = Difference(Intersection(cube, ball), Union(drills));

Compiling and using it on the host
----------------------------------

A `Tape <doxygen/html/classEBGeometry_1_1Tape.html>`__ compiles the expression into a
`Pool <doxygen/html/classEBGeometry_1_1Pool.html>`__, which must outlive the tape and every copy of it
(see :ref:`Chap:MemoryModel`):

.. code-block:: c++

   Pool pool(hostMemoryResource());

   const Tape<T> tape(pool, solid);
   const T       value = tape.signedDistance(point);   // negative inside, positive outside

An intersection or a difference of exact distances is only a lower bound on the distance, which
``getDistanceQuality()`` reports. ``closestPrimitive(point)`` names the leaf the value comes from (see
:ref:`Chap:ClosestPrimitive`), numbered in expression order: the cube 0, the ball 1, the drills 2 to 4.

The same query on a GPU
-----------------------

The query is a trivially copyable functor holding the tape by value. The program freezes the pool,
mirrors it into ``GPU::memoryResource()`` (device memory in a CUDA or HIP build), and runs the query
on a view of the tape onto the copy:

.. code-block:: c++

   pool.freeze();

   const Pool devicePool = Pool::mirror(pool, GPU::memoryResource());

   GPU::transform(Distance{tape.rebasedView(devicePool)}, points, values);

In a plain C++ build the same lines run on the host.

Looking at it
-------------

The example ends with ``Render::view(tape, pool, argc, argv)``, a window on the solid.

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit and scroll to zoom; the window lists its other keys (see
:ref:`Sec:ViewerKeys`).

Building and running
--------------------

See :ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler invocation,
and :ref:`Sec:ExamplesGPU` for a GPU build.

.. code-block:: bash

   cd Examples/BasicCSG
   ./BasicCSG.ex
