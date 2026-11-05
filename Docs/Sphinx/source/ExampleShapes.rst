.. _Chap:ExampleShapes:

Shapes
=======

The first example to read. It constructs every built-in analytic shape (see
:ref:`Sec:AnalyticShapes`), evaluates each one, and runs one of them through ``GPU::transform``.
The source is at :file:`Examples/Shapes/main.cpp`.

.. figure:: /_static/example_Shapes.jpg
   :width: 60%
   :align: center

   The bounded shapes, as the viewer first shows them; the hollow shell is turned over to show its inside.

The code that runs on either side
---------------------------------

The file opens with the query that the last section hands to ``GPU::transform``: a small struct
holding a shape by value, whose ``EBGEOMETRY_HOST_DEVICE`` call operator returns the shape's signed
distance at a point. A shape is a trivially copyable value, so this is all a kernel needs.

Construct the shapes
--------------------

Each shape is one constructor call, with the shape's position before its sizes:
``SphereSDF<T>(center, radius)``, ``BoxSDF<T>(lo, hi)``, ``TorusSDF<T>(center, majorRadius,
minorRadius)``, ``TubeSDF<T>(center1, center2, innerRadius, outerRadius)``, and so on. A shape with a
built-in axis (the torus, the cones, the rounded cylinder, the capped torus, the cut spheres, the
wedge) has it along :math:`y`; the cylinder, capsule, capped and round cones, prism and tube take the
centers of their two ends, so they can point anywhere. A size is that of the finished shape, so a
rounded box of outer size 1.2 fits a box of side 1.2 exactly. The shapes other than the rounded box and
cylinder have sharp edges; the example rounds a hexagonal prism by taking ``Fused::Offset`` of a slightly
smaller one. The parameters of every shape are documented in
`EBGeometry_AnalyticDistanceFunctions.hpp <doxygen/html/EBGeometry__AnalyticDistanceFunctions_8hpp.html>`__.
The example also constructs `PerlinSDF <doxygen/html/classEBGeometry_1_1PerlinSDF.html>`__, a fractal
noise field: not the distance to any surface, but a common building block for perturbing one.

Use them on the host
--------------------

The program prints one row per shape, using the three calls every shape has:

* ``signedDistance(point)``: negative inside, positive outside, zero on the surface.
* ``computeBoundingVolume()``: the shape's axis-aligned bounding box, infinite along any direction
  in which the shape is unbounded (the plane, the infinite cylinder and cone, the noise).
* ``distanceQuality``: whether the value is the exact Euclidean distance, a lower bound on it, or
  only a sign (`DistanceQuality <doxygen/html/EBGeometry__DistanceQuality_8hpp.html>`__). Every
  built-in shape is exact except the noise; the ellipsoid's exact distance comes from a short
  iteration, so it costs more to evaluate than the others. Composition and BVH pruning depend on it; see
  :ref:`Chap:ExampleBasicCSG`.

.. code-block:: text

   shape                 (0,0,0) (.5,.5,.5)    (2,0,0)   box along x   quality
   plane                  0.0000     0.8660     1.1547   [-inf, +inf]   exact
   sphere                -1.0000    -0.1340     1.0000   [-1.00, 1.00]   exact
   ...
   Perlin noise           0.9990     1.0615     0.9990   [-inf, +inf]   not a distance

The same query on a GPU
-----------------------

The last section evaluates the torus at points along the :math:`x` axis with
``GPU::transform(Distance{torus}, points, distances)``, which writes into a vector the caller has
sized. Compiled as CUDA or HIP, that runs a kernel; compiled as
plain C++, a host loop, so the section runs in both builds. A shape owns no memory, so nothing but
the points has to reach the device. See :ref:`Chap:RunningOnGPU`.

Looking at it
-------------

The example ends with a window on the sixteen bounded shapes side by side, compiled into one tape (see
:ref:`Chap:ExampleBasicCSG`) with each moved to its place by ``Translate``.

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit and scroll to zoom; the window lists its other keys (see
:ref:`Sec:ViewerKeys`).

Building and running
--------------------

See :ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler invocation,
and :ref:`Sec:ExamplesGPU` for a GPU build.

.. code-block:: bash

   cd Examples/Shapes
   ./Shapes.ex
