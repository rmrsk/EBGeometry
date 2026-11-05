.. _Chap:ExampleSphereClouds:

Sphere clouds
=============

Particles of differing sizes packed in a cylinder, kept as a center and a radius each in arrays of the program's own:
the signed distance to their union, and the particle closest to a point, through a
`SphereCloudBVH <doxygen/html/classEBGeometry_1_1SphereCloudBVH.html>`__ (see :ref:`Chap:ImplemPointCloud`); then the
cylinder with the particles cut out of it, fused and in a tape, and a ray through both. The source is at
:file:`Examples/SphereClouds/main.cpp`.

.. figure:: /_static/example_SphereClouds.jpg
   :width: 60%
   :align: center

   2,400 particles with radii between 0.01 and 0.035 packed in a cylinder (left), and the cylinder with them cut
   out of it (right).

The code that runs on either side
---------------------------------

The query functor ``ClosestParticle`` holds the cloud by value and maps a point to the closest particle, through
an ``EBGEOMETRY_HOST_DEVICE`` call operator. A cloud owns no memory -- it describes the spheres and the BVH in a
``Pool`` -- so it is trivially copyable, and a kernel can receive it by value once it describes memory the device
can read.

Packing the particles
---------------------

The particles are placed one at a time, largest first, each at the first random position in the cylinder where it
overlaps no particle placed before it: a loose random packing. A center can lie near the wall, so a particle can
stick out of the cylinder, and leaves a pit in its wall when cut out.

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

The cylinder with the particles cut out
---------------------------------------

The cloud is a shape, so it takes part in CSG like any other (see :ref:`Chap:ImplemCSG`): here the cylinder minus
the particles, a porous solid. A fused operation fixes the composition at compile time, and is the faster where the
composition is known then; a tape holds a composition built at run time, and lists the cloud's type among its
primitives. Both live in the cloud's pool:

.. code-block:: c++

   const auto porous = Fused::Difference(cylinder, cloud);

   using Porous = Tape<T, SphereCloudBVH<T>>;

   const Porous porousTape(pool, Difference(Expr<T>(cylinder), Expr<T>(cloud)));

A ray crosses the cloud exactly, through its BVH, and reports the particle it meets in ``HitT::m_primitive``; where
particles overlap, it crosses only the surface of their union. Through either composition it is just as exact. The
program sends one ray through the cylinder and prints where it meets a particle and where it meets the porous
cylinder, fused and through the tape.

The same query on a GPU
-----------------------

The pool is frozen and mirrored into ``GPU::memoryResource()``, and ``rebasedView()`` gives a cloud that reads the
copy; ``GPU::transform()`` runs ``ClosestParticle`` once per point in a kernel. In a plain C++ build the same lines
run on the host.

Looking at it
-------------

The scene is the particles and the porous cylinder side by side, one fused union,
``Fused::Union(cloud, Fused::Translate(porous, shift))``. The example ends with
``Render::view(scene, pool, argc, argv)``, which traces the particles exactly; X cuts the scene open.

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
