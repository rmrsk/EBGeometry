.. _Chap:ExampleTornado:

Tornado
=======

How a union of very many spheres scales: a tornado of half a million spheres of one radius (try eight million), packed
without overlapping and colored by the speed of the wind where each one sits. The spheres are sorted into 16 bins of
speed; each bin is a `BVHUnion <doxygen/html/classEBGeometry_1_1BVHUnion.html>`__ of its spheres, and the bins are the
leaves of one `Tape <doxygen/html/classEBGeometry_1_1Tape.html>`__, a union through a BVH over them (see
:ref:`Sec:BVHUnions`). The source is at :file:`Examples/Tornado/main.cpp`.

.. figure:: /_static/example_Tornado.jpg
   :width: 60%
   :align: center

   500,000 spheres in a funnel, its cloud base and the debris rising around its foot, colored by wind speed: rust
   where it is slow, bone white where it is fast.

Packing the spheres
-------------------

The spheres are placed as in :ref:`Chap:ExampleSphereClouds`, each at the first random position where it overlaps no
sphere placed before it, here in the shape of a tornado: a funnel that widens and sways as it rises, with spiral bands
winding up it, under a cloud base and over an inverted cone of debris around its foot. Comparing a new sphere with every
sphere placed before it costs as the square of the count, far too much for millions, so the program keeps a hash grid
of cells one diameter wide, and compares it only with the spheres in its own cell and the neighboring ones.

The wind speed at each sphere comes from a model of a tornado's core (a Burgers-Rott vortex): a swirl that is fastest
on the funnel's wall and falls off outside it, an updraft up the core, and inflow along the ground. It is kept beside
the sphere's center, in an array of the program's own.

Why 16 bins
-----------

A tape is colored by leaf: the viewer gives each leaf of a tape its own color (``ColorBy::Primitive``, with
``Render::Settings::m_primitiveColors``, which has 16 entries). A ``BVHUnion`` is one leaf, however many spheres it
holds, so one union of every sphere would show in one color. The spheres are therefore sorted into 16 bins of speed,
equal steps from the slowest sphere to the 99th percentile (the fastest few, at the very foot of the funnel, join the
last bin), and each bin becomes a ``BVHUnion`` of its own. The bins are the operands of one union in a tape, which
lists the ``BVHUnion`` type among its primitives and numbers its leaves in the order of the operands, so leaf ``i`` is
bin ``i`` and takes the ``i``-th color of the colormap:

.. code-block:: c++

   using Bin     = BVHUnion<T, BVH::DefaultBranchingRatio<T>(), SphereSDF<T>>;
   using Tornado = Tape<T, Bin>;

   std::vector<Expr<T>> leaves;

   for (int b = 0; b < 16; b++) {
     settings.m_primitiveColors[leaves.size()] = desert(T(b) / T(15));
     leaves.emplace_back(Bin(pool, bins[b]));
   }

   const Tornado tornado(pool, Union(leaves, Accelerate::BVH));

Both levels are BVHs: each ``BVHUnion`` builds one over its spheres, and ``Accelerate::BVH`` builds the tape's union
through one over the bins' boxes. The bins are not apart in space -- every bin of speed winds through the whole
tornado, so their boxes overlap -- and a query may descend into several of them; within a bin, its own BVH finds the
closest sphere among hundreds of thousands. The program prints each bin's speed and size, and how long the packing and
the build take, so the cost of more spheres can be read from one run to the next.

Looking at it
-------------

The example ends with ``Render::view(tornado, pool, camera, settings, argc, argv)``, the camera fitted with y up and
the settings coloring by leaf. X cuts the tornado open.

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit and scroll to zoom; the window lists its other keys (see
:ref:`Sec:ViewerKeys`).

Building and running
--------------------

See :ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler invocation.
Built for a GPU (see :ref:`Sec:ExamplesGPU`), the window renders on the device.

.. code-block:: bash

   cd Examples/Tornado
   ./Tornado.ex                                  # 500,000 spheres
   ./Tornado.ex --count=8000000                  # 8 million
   ./Tornado.ex --count=8000000 --radius=0.002   # thinner

``--count`` sets the number of spheres and ``--radius`` their radius. By default the radius follows the count, so that
the tornado looks as dense at any count. A sphere that finds no room is left out, so the program prints how many it
placed.
