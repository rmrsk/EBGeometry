.. _Chap:ExampleSphereClouds:

Sphere clouds
=============

Particles of differing sizes, kept as a center and a radius each in arrays of the program's own: the signed
distance to their union, and the particle closest to a point, through a
`SphereCloudBVH <doxygen/html/classEBGeometry_1_1SphereCloudBVH.html>`__ (see :ref:`Chap:ImplemPointCloud`); then the
cloud in a tape, cut out of a block, and a ray through both. The source is at
:file:`Examples/SphereClouds/main.cpp`.

.. figure:: /_static/example_SphereClouds.jpg
   :width: 60%
   :align: center

   A block with 2,000 particles cut out of it, with radii between 0.01 and 0.06.

The code that runs on either side
---------------------------------

The query functor ``ClosestParticle`` holds the cloud by value and maps a point to the closest particle, through
an ``EBGEOMETRY_HOST_DEVICE`` call operator. A cloud owns no memory -- it describes the spheres and the BVH in a
``Pool`` -- so it is trivially copyable, and a kernel can receive it by value once it describes memory the device
can read.

Building the cloud
------------------

The cloud is built from the program's arrays of centers and radii (or from one array of
``SphereCloud::Sphere``), and keeps its own copy in the pool, which must outlive it:

.. code-block:: c++

   EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

   const SphereCloudBVH<T> cloud(pool, centers, radii);

A query reports a particle by its index in those arrays, so data the cloud does not hold (here a material per
particle) stays in the program's own arrays, indexed the same way.

Using it on the host
--------------------

``signedDistance(p)`` is the smallest :math:`|p - c| - r` over the particles: negative inside a particle, and the
distance to the nearest one outside them all. ``closestSphere(p)`` names that particle and gives the same
distance. The program prints both at a few points, and the value at a particle's center.

The cloud in a tape
-------------------

The cloud is a shape, so it takes part in CSG through a tape (see :ref:`Chap:ImplemCSG`), as a mesh SDF does: its
type is listed among the tape's primitives, and the tape is built in the cloud's pool. Here a block with the
particles cut out of it:

.. code-block:: c++

   using Block = EBGeometry::Tape<T, SphereCloudBVH<T>>;

   const Block block(pool, Difference(Expr<T>(box), Expr<T>(cloud)));

A ray crosses the cloud exactly, through its BVH, and reports the particle it meets in ``HitT::m_primitive``; where
particles overlap, it crosses only the surface of their union. Through the tape it is just as exact. The program
sends one ray up through the block and prints where it meets a particle and where it meets the block.

The same query on a GPU
-----------------------

The pool is frozen and mirrored into ``GPU::memoryResource()``, and ``rebasedView()`` gives a cloud that reads the
copy; ``GPU::transform()`` runs ``ClosestParticle`` once per point in a kernel. In a plain C++ build the same lines
run on the host.

Looking at it
-------------

The example ends with ``Render::view(block, pool, argc, argv)``, which traces the particles exactly; X cuts the block
open.

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit and scroll to zoom; the window lists its other keys (see
:ref:`Sec:ViewerKeys`).

Building and running
--------------------

See :ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler invocation.
Built for a GPU (see :ref:`Sec:ExamplesGPU`), the queries and the window run on the device.

.. code-block:: bash

   cd Examples/SphereClouds
   ./SphereClouds.ex
