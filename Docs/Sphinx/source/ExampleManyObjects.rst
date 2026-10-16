.. _Chap:ExampleManyObjects:

ManyObjects
===========

The signed distance to a scene of many objects is the smallest signed distance to any of them.
Computed literally, every query visits every object, so the cost grows with the number of objects.
This example builds two such scenes, 8,000 spheres on a lattice and a "city" of 3,600 randomly sized
boxes, and evaluates each in two ways: a naive loop over every object, and a
`BVHUnion <doxygen/html/classEBGeometry_1_1BVHUnion.html>`__, which evaluates the same union through
a bounding volume hierarchy over the objects' boxes (see :ref:`Chap:BVH`). It times both. Its last
section runs the unions on a GPU and shows how to time the queries there without timing the memory
traffic around them.

The source is at :file:`Examples/ManyObjects/main.cpp`. See :ref:`Chap:Building` for how to compile
it with CMake, GNU Make, or a direct compiler invocation.

.. code-block:: bash

   cd Examples/ManyObjects
   ./ManyObjects.ex

The query
---------

The query is a small functor whose ``EBGEOMETRY_HOST_DEVICE`` call operator returns the union's
signed distance at a point. It holds the union by value. A ``BVHUnion`` is a descriptor of the
objects and the BVH it stored in a `Pool <doxygen/html/classEBGeometry_1_1Pool.html>`__, so the copy
is cheap, and a kernel can receive it once it describes a pool the device can read.

Building the scenes
-------------------

A ``BVHUnion`` holds one primitive type, here
`SphereSDF <doxygen/html/classEBGeometry_1_1SphereSDF.html>`__ for one scene and
`BoxSDF <doxygen/html/classEBGeometry_1_1BoxSDF.html>`__ for the other. Its constructor takes the
pool, the objects and one bounding box per object:

.. code-block:: c++

   EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

   const EBGeometry::BVHUnion<T, Box, K> city(pool, buildings, buildingBoxes);

The union copies the objects into the pool and builds its BVH there, so the input vectors are not
needed afterwards. The pool must outlive the union and every copy of it. To combine objects of
different types in one union, compose them with the expression builder instead
(:ref:`Chap:ExampleCSG`).

Using the unions on the host
----------------------------

``signedDistance(point)`` gives the same value as the naive minimum over every object: the BVH only
skips objects whose boxes show that they cannot be the closest. For each scene the program prints the
time per query point of both, and the speedup, which grows with the number of objects. Change
``spheresPerAxis``, ``lotsPerAxis`` and ``numPoints`` at the top of the file to see this.
``computeBoundingVolume()`` gives the box around the whole union, which places the random query
points.

Timing the queries on a GPU
---------------------------

The pool holding both unions is frozen, mirrored into ``GPU::memoryResource()`` (device memory in a
CUDA or HIP build) and each union rebased onto the copy with ``rebasedView()``, as in
:ref:`Chap:ExampleMeshSDF`.

The one-call
`GPU::transform(query, points, results) <doxygen/html/EBGeometry__GPUTransform_8hpp.html>`__ takes
the points in a host ``std::vector`` and writes the results into another. In a GPU build, every call allocates
device memory, copies the points in, runs the kernel, copies the results out and frees the memory, so
timing it times all of that. To time the queries alone, the example puts the points, and room for the
results, on the device first. Both are
`PODVector <doxygen/html/structEBGeometry_1_1PODVector.html>`__\ s in a pool of their own, mirrored
once:

.. code-block:: c++

   in.reserveFrom(hostPool, n);
   out.reserveFrom(hostPool, n);
   in.assign(hostPool.base(), points.data(), n);
   out.assign(hostPool.base(), zeros.data(), n);
   hostPool.freeze();

   const Pool devicePool = Pool::mirror(hostPool, GPU::memoryResource());

   GPU::transform(query, devicePool, in, out);

A ``PODVector`` is an offset and a size, so the pool overload of ``GPU::transform`` takes the pool
both arrays live in. It checks that the pool is frozen and in memory the device can reach, then
reads and writes the arrays where they are, with no allocation and no copies. Mirroring
``devicePool`` back to host memory then makes the results readable. (The span overload, over
`PODSpan <doxygen/html/structEBGeometry_1_1PODSpan.html>`__\ s from ``in.bind(devicePool.base())``,
does the same without the check.)

The program prints both times per point. On a GPU the gap between them is the copying. In a plain
C++ build the section runs on the host, where ``GPU::memoryResource()`` is host memory standing in
for a device and ``GPU::transform`` is a loop, so the two times are about the same. In a GPU build
the section uses about a million points, so that the time is the kernel's rather than the launch's;
on the host it keeps ``numPoints``. See :ref:`Sec:ExamplesGPU` for building for a GPU.
