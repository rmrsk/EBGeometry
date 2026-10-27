.. _Chap:ExampleTwist:

Twist
=====

An operation of your own: a twist about the :math:`y` axis, applied to a mesh read from a file. The source is at :file:`Examples/Twist/main.cpp`.

.. figure:: /_static/example_Twist.jpg
   :width: 60%
   :align: center

   The armadillo and its twisted copy.

.. important::

   The default mesh comes from the ``common-3d-test-models`` git submodule. Clone with
   ``--recurse-submodules``, or run ``git submodule update --init`` in the repository (see
   :ref:`Sec:Cloning`); otherwise pass a mesh of your own as the first argument.

Writing the operation
---------------------

``Twisted<S>`` is a trivially copyable struct holding a shape ``S`` (here a ``Tape`` of the placed mesh;
any shape works) and a twist rate. Its ``signedDistance(p)`` turns ``p`` back about the :math:`y` axis by
the rate times ``p[1]`` and evaluates the shape there; ``computeBoundingVolume()`` is the shape's box swept
around the axis. Like any shape of your own (see :ref:`Chap:ExampleCustomShape`), it also says how far
its value can be trusted:

.. code-block:: c++

   static constexpr DistanceQuality distanceQuality = DistanceQuality::Bound;

A twist is not a rigid motion: at distance :math:`r` from the axis it stretches lengths by up to
:math:`\sqrt{1 + (kr)^2}` for a rate :math:`k`. Sphere tracing (the renderer) and BVH pruning evaluate a shape
only inside its box, a square of half-width :math:`R` about the axis whose corners are :math:`\sqrt{2}R`
from it; dividing the value by the factor at the corners, :math:`\sqrt{1 + 2(kR)^2}`, keeps it a lower
bound on the distance there, which is what they need.

Using it
--------

The program prints the original's and the twisted copy's values at three heights; at :math:`y = 0` the
twist turns nothing and the two differ only by the factor. A type of your own goes into a tape listed
after ``T``:

.. code-block:: c++

   const Tape<T, Twisted<Tape<T>>> scene(pool, Union(placed, Translate(Expr<T>(twisted), Vec3(1.6, 0, 0))));

A twist built into the library, with the tape's and the fused operations' arithmetic and a ray form, is
on the roadmap.

Looking at it
-------------

The example ends with ``Render::view(scene, argc, argv)``, a window on the armadillo and its twisted copy.
The twisted copy has no ray form, so the renderer sphere traces it, which the bound makes safe.

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit, scroll to zoom, X cuts the object open and P colours each primitive; see
:ref:`Chap:ImplemRender`.

Building and running
--------------------

See :ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler invocation.
It runs on the host only.

.. code-block:: bash

   cd Examples/Twist
   ./Twist.ex
