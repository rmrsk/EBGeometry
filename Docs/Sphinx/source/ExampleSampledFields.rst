.. _Chap:ExampleSampledFields:

Sampled fields
==============

Distance fields given as samples on a grid, and a terrain given as heights, both as a
`SampledGridSDF <doxygen/html/classEBGeometry_1_1SampledGridSDF.html>`__, queried on the host and on a GPU. The
source is at :file:`Examples/SampledFields/main.cpp`.

.. figure:: /_static/example_SampledFields.jpg
   :width: 60%
   :align: center

   The sampled peanut, and the block of terrain beside it.

The code that runs on either side
---------------------------------

The file opens with the query that the GPU section hands to ``GPU::transform``: a small struct holding the grid by
value, whose ``EBGEOMETRY_HOST_DEVICE`` call operator returns its signed distance at a point. The grid is a
descriptor of its samples in a pool, as a mesh distance field is of its mesh, so it is all a kernel needs once it
describes a pool the device can read.

A field sampled on a grid
-------------------------

The samples stand for a scan: the distance to two blended balls at the nodes of a :math:`48^3` grid, :math:`x`
fastest, then :math:`y`, then :math:`z`, negative inside. Your own come from a file or another code. The
constructor takes the pool, the position of the first node, the spacing along each axis, the number of nodes along
each, and the samples. The grid interpolates them trilinearly and divides them by their largest slope, so its value
never exceeds the distance to its surface: a ``Bound``, which keeps sphere tracing and the pruning of a BVH union
safe. The solid is where the field is negative, inside the grid's box; outside the box, the value grows with the
distance to it.

A height field
--------------

``SampledGridSDF<T>::heightField(pool, corner, dx, dz, nx, nz, heights)`` takes heights on a grid in the
:math:`xz`-plane, :math:`x` fastest, with the floor's height as the corner's :math:`y`. The solid is a block of
terrain between the floor and the surface, its sides cut by the grid's box. It is the same type: a grid of two layers
of nodes whose interpolation is the height above the surface.

Use them on the host
--------------------

Both are shapes like any other: ``signedDistance()``, ``computeBoundingVolume()`` and ``distanceQuality``. The
program prints the peanut's value at three points and the terrain's above and below its surface, then compiles the
two side by side into a ``Tape`` for the window. A tape holds a grid without being told its type; the grid must live
in the tape's pool.

The same query on a GPU
-----------------------

The program freezes the pool, mirrors it into ``GPU::memoryResource()`` (device memory in a CUDA or HIP build) and
runs the query on a view of the grid on the mirror, ``peanut.rebasedView(devicePool)``, through ``GPU::transform``.
In a plain C++ build the same lines run as a host loop. See :ref:`Chap:RunningOnGPU`.

Looking at it
-------------

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit and scroll to zoom; the window lists its other keys (see
:ref:`Sec:ViewerKeys`).
