.. _Chap:ExampleOctreeBoundingVolume:

OctreeBoundingVolume
======================

Estimating a tight bounding box, by octree refinement, for anything that gives a distance at a point
(see :ref:`Sec:OctreeBoundingVolume`): first for one analytic shape, then for a union of three. The
estimate needs no memory of its own, so the last section also runs it in a GPU kernel. The source is
at :file:`Examples/OctreeBoundingVolume/main.cpp`, in the following sections.

The code that runs on either side
---------------------------------

``ShapeUnion`` is the union, the pointwise minimum, of a sphere, a torus around it and a capsule
poking out of it, whose combined extent is not obvious from any one formula. It is a struct rather
than a lambda: a function that reaches a GPU kernel must be trivially copyable with an
``EBGEOMETRY_HOST_DEVICE`` call operator, and the same struct serves the host.

``EstimateBox`` is the query for the GPU section. It holds the union and the starting box by value,
and maps an octree depth to the union's estimated box at that depth.

Building the shapes
-------------------

A finite cone (tip at the origin, height 2, a 60 degree opening angle) and the union of the sphere,
torus and capsule above.

Using it on the host
--------------------

`approximateBoundingVolumeOctree(f, lo, hi, depth, margin)
<doxygen/html/EBGeometry__FunctionQueries_8hpp.html>`__ starts from the box ``[lo, hi]``, which must
hold the surface, splits it into eight cells, keeps the cells the surface can pass through, and
repeats down to the given depth. The result is the box around the kept cells: it holds the surface and
overshoots it by at most one leaf cell. The margin pads the intersection test; 0 means none. ``f`` can
be any shape, or anything else that returns a distance for a point:

.. code-block:: c++

   const AABB coneBox  = approximateBoundingVolumeOctree(cone,   initLo, initHi, maxDepth, T(0));
   const AABB unionBox = approximateBoundingVolumeOctree(shapes, initLo, initHi, maxDepth, T(0));

The search starts from a deliberately loose box of :math:`\pm 10` in each direction and refines for 8
levels. The program prints each estimate next to the exact box the analytic shapes give from
``computeBoundingVolume()``.

The same query on a GPU
-----------------------

`GPU::transform(query, depths) <doxygen/html/EBGeometry__GPUTransform_8hpp.html>`__ runs one estimate
per depth from 1 to 8: in a kernel when the file is compiled as CUDA or HIP, and in a host loop
otherwise (see :ref:`Sec:ExamplesGPU`). The shapes are plain values inside the query, so nothing needs
copying to the device first. The printed boxes tighten as the depth grows.

Building and running
--------------------

See :ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler
invocation.

.. code-block:: bash

   cd Examples/OctreeBoundingVolume
   ./OctreeBoundingVolume.ex
