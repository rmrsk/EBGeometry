.. _Chap:ExampleTwist:

Twist
=====

An operation of your own: a twist about the :math:`y` axis, applied to a mesh read from a file. The
source is at :file:`Examples/Twist/main.cpp`.

.. figure:: /_static/example_Twist.jpg
   :width: 60%
   :align: center

   The twisted armadillo.

.. important::

   The default mesh comes from the ``common-3d-test-models`` git submodule. Clone with
   ``--recurse-submodules``, or run ``git submodule update --init`` in the repository (see
   :ref:`Sec:Cloning`); otherwise pass a mesh of your own as the first argument.

Writing the operation
---------------------

``Twisted<S>`` is a trivially copyable struct holding a shape ``S`` (here a ``Tape`` of the placed mesh;
any shape works) and a twist rate. Its ``signedDistance(p)`` turns ``p`` back about the :math:`y` axis by
the rate times ``p[1]`` and evaluates the shape there; ``computeBoundingVolume()`` is the shape's box
swept around the axis. Like any shape of your own (see :ref:`Chap:ExampleCustomShape`), it also says how
far its value can be trusted:

.. code-block:: c++

   static constexpr DistanceQuality distanceQuality = DistanceQuality::Bound;

A twist is not a rigid motion: at distance :math:`r` from the axis it shears by :math:`a = kr` for a rate
:math:`k`, which stretches lengths by up to :math:`(a + \sqrt{a^2 + 4})/2`, the largest singular value of
the shear. Sphere tracing (the renderer) and BVH pruning evaluate a shape only inside its box, a square of
half-width :math:`R` about the axis whose corners are :math:`\sqrt{2}R` from it; dividing the value by the
factor there, with :math:`a = \sqrt{2}kR`, keeps it a lower bound on the distance, which is what they
need.

Using it
--------

The program prints the original's and the twisted copy's values at three heights; at :math:`y = 0` the
twist turns nothing and the two differ only by the factor. Like any shape of your own, the twisted
statue can also go into a tape, listed after ``T`` (see :ref:`Chap:ExampleCustomShape`). The library
has no built-in twist operation yet (none in the tape, the fused operations or the ray forms), which is
why this example defines its own.

Looking at it
-------------

The example ends with ``Render::view(twisted, pool, argc, argv)``, a window on the twisted armadillo. It
has no ray form, so the renderer sphere traces it, which the bound makes safe. In a GPU build the window
renders on the device, through ``rebasedView()``: ``Twisted`` gives the view of its shape onto a mirror
of the pool, as the library's own wrappers do.

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit and scroll to zoom; the window lists its other keys (see
:ref:`Sec:ViewerKeys`).

Building and running
--------------------

See :ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler invocation.
Built for a GPU (see :ref:`Sec:ExamplesGPU`), the window renders on the device.

.. code-block:: bash

   cd Examples/Twist
   ./Twist.ex
