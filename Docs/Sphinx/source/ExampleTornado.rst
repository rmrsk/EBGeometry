.. _Chap:ExampleTornado:

Tornado
=======

A property carried on a sphere cloud, at scale: a tornado of half a million spheres (try eight million), each carrying
the speed of the wind where it sits. The spheres are kept in one
`SphereCloudBVH <doxygen/html/classEBGeometry_1_1SphereCloudBVH.html>`__ (see :ref:`Sec:SphereCloudBVH`), and their
speeds in an array of the program's own, put in the same pool. A query asks the wind at a point, on the host and on a
GPU, and the window colors every sphere by its speed. The source is at :file:`Examples/Tornado/main.cpp`.

.. figure:: /_static/example_Tornado.jpg
   :width: 60%
   :align: center

   500,000 spheres in a funnel, its cloud base and the debris around its foot, colored by wind speed: rust where it
   is slow, bone white where it is fast; the floor reflects them.

Packing the spheres
-------------------

The spheres are placed as in :ref:`Chap:ExampleSphereClouds`, each at the first random position where it overlaps no
sphere placed before it, here in the shape of a tornado: a funnel that widens and sways as it rises, with spiral bands
winding up it, under a cloud base and over an inverted cone of debris around its foot. Comparing a new sphere with every
sphere placed before it costs as the square of the count, far too much for millions, so the program keeps a hash grid
of cells one diameter wide, and compares it only with the spheres in its own cell and the neighboring ones.

The wind speed at each sphere comes from a model of a tornado's core (a Burgers-Rott vortex): a swirl that is fastest
on the funnel's wall and falls off outside it, an updraft up the core, and inflow along the ground. It is kept in an
array beside the centers, in the same order.

A property beside the cloud
---------------------------

A sphere carries no data of its own: the cloud stores centers and radii, and its queries name a sphere by its *cloud
index*, its position in the arrays the cloud was built from. Anything else known about a sphere (a speed, a material,
a temperature) is an array of the program's own in the same order, read at that index. To read it on a GPU as well,
the array goes in the cloud's pool, as a
`PODVector <doxygen/html/structEBGeometry_1_1PODVector.html>`__, so that it is mirrored to the device with the cloud:

.. code-block:: c++

   const SphereCloudBVH<T> cloud(pool, centers, radii);

   PODVector<T> speedsInPool;

   speedsInPool.reserveFrom(pool, speeds.size());
   speedsInPool.assign(pool.base(), speeds.data(), speeds.size());

The program's ``WindyCloud`` holds the two together, the cloud and the speeds, both descriptors of the one pool. It is
trivially copyable, ``speedOf(index)`` reads a sphere's speed from wherever the pool is, and ``rebasedView()`` gives
the pair that reads another copy of the pool.

The code that runs on either side
---------------------------------

The query functor ``WindAt`` holds a ``WindyCloud`` by value and gives the wind at a point: the speed of the closest
sphere, ``speedOf(cloud.closestSphere(p).index)``, through an ``EBGEOMETRY_HOST_DEVICE`` call operator.

Using it on the host
--------------------

At a few points on the funnel's wall, from its foot to its mouth, the program asks the cloud for the closest sphere
and prints its index and the speed its own array gives there: fastest at the narrow foot, slower up the funnel.

The same query on a GPU
-----------------------

The pool is frozen and mirrored into ``GPU::memoryResource()``, and ``rebasedView()`` gives a ``WindyCloud`` that reads
the copy, the cloud and the speeds both; ``GPU::transform()`` runs ``WindAt`` once per point in a kernel, and gives the
same speeds. In a plain C++ build the same lines run on the host.

Coloring by the property
------------------------

The window colors a shape by primitive with ``ColorBy::Primitive``, one of 16 colors
(``Render::Settings::m_primitiveColors``) for each id a ray reports. A ray on the cloud reports the cloud index of the
sphere it hits, so ``WindyCloud`` is a shape of its own (a signed distance and a box, both the cloud's) with a ray
form, a specialization of `RayForm <doxygen/html/structEBGeometry_1_1RayForm.html>`__ (see :ref:`Chap:ImplemRays`). It
calls the cloud's ray form, which traces the cloud exactly through its one BVH, and reports the sphere hit by the step
of speed it falls in, from 0 to 15, instead of its index:

.. code-block:: c++

   HitT<T> hit = RayForm<SphereCloudBVH<T>>::firstHit(shape.m_cloud, ray, tMin, tMax, options);

   if (hit.hit()) {
     hit.m_primitive = shape.colorOf(hit.m_primitive);
   }

The steps span the speeds from the slowest sphere to the 99th percentile; the fastest few, at the very foot of the
funnel, take the last color. ``closestLeaf()`` does the same for a point, for the section plane. The lookup is one
read per ray that hits, so the colors cost next to nothing beside the trace.

Looking at it
-------------

The example ends with ``Render::view(tornado, pool, camera, settings, argc, argv)``, the camera fitted with y up and
the settings coloring by primitive, in the mirror look, ``Render::setLook(settings, Render::Look::Mirror)``: a dark
backdrop, and a floor that reflects the tornado (see :ref:`Chap:ImplemRender`). B and Shift + B cycle the other looks,
Up and Down brighten or dim the light, Left and Right the ambient light, and X cuts the tornado open.

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit and scroll to zoom; the window lists its other keys (see
:ref:`Sec:ViewerKeys`).

Building and running
--------------------

See :ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler invocation.
Built for a GPU (see :ref:`Sec:ExamplesGPU`), the query and the window run on the device.

.. code-block:: bash

   cd Examples/Tornado
   ./Tornado.ex                                  # 500,000 spheres
   ./Tornado.ex --count=8000000                  # 8 million
   ./Tornado.ex --count=8000000 --radius=0.002   # thinner

``--count`` sets the number of spheres and ``--radius`` their radius. By default the radius follows the count, so that
the tornado looks as dense at any count. A sphere that finds no room is left out, so the program prints how many it
placed.
