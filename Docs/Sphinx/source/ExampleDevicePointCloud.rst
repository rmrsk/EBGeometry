.. _Chap:ExampleDevicePointCloud:

DevicePointCloud
================

k-nearest-neighbor queries on a ``PointCloudBVH`` (see :ref:`Chap:ImplemPointCloud`), run in a GPU
kernel. The same source builds as plain C++, where the same queries run in a host loop instead.

**Memory.** The cloud is built on the host. In a GPU build its pool is frozen and mirrored to device
memory in one piece, and the cloud is rebased onto the copy:

.. code-block:: c++

   Pool pool(hostMemoryResource());
   const PointCloudBVH<T> cloud(pool, positions);
   pool.freeze();
   const Pool devicePool = Pool::mirror(pool, deviceMemoryResource());
   const PointCloudBVH<T> deviceCloud = cloud.rebasedView(devicePool);  // pass this to a kernel

This is the pattern for large geometry: the device reads its own copy at full speed (see
:ref:`Chap:MemoryModel`). For a small object, managed memory saves the copy; see
:ref:`Chap:ExampleDeviceShape`.

**The query.** A trivially copyable functor holds the rebased cloud by value, and its
``EBGEOMETRY_HOST_DEVICE`` call operator returns a point's 4 nearest cloud points from
``closestPoints()``. A few lines of kernel run it once per query point on the device, and in a plain
C++ build a loop runs it on the host.

The cloud is 100,000 random points in the unit cube plus eight planted points at the corners of the
cube :math:`[-1, 2]^3`, each far from every other point. A query next to a planted point must find
that point first, at a known distance. The 4 nearest points of every query, the planted ones and
1,000 random ones, must match the library's brute-force scan, ``PointCloud::closestPointsBruteForce()``.
The program exits with a nonzero status if any differ.

The source for this example is at :file:`Examples/DevicePointCloud/main.cpp`. See
:ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler invocation, and
:ref:`Sec:ExamplesGPU` for the GPU builds.

.. code-block:: bash

   cd Examples/DevicePointCloud
   make                  # plain C++: the queries run in a host loop
   make clean
   make GPU=HIP          # or GPU=CUDA: the queries run in a kernel
   ./DevicePointCloud.ex
