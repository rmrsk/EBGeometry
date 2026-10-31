.. _Chap:ExamplePointClouds:

PointClouds
===========

Searching a point cloud with the turnkey
`PointCloudBVH <doxygen/html/classEBGeometry_1_1PointCloudBVH.html>`__ and
`PointCloudHashGrid <doxygen/html/classEBGeometry_1_1PointCloudHashGrid.html>`__ classes (see
:ref:`Chap:ImplemPointCloud`). The program answers two kinds of question: the closest points to an
arbitrary query point, and the nearest neighbors of points that are themselves in the cloud (the
k-nearest-neighbor graph). The two classes answer both through the same interface, so the program
runs the same code once with each.

The source is at :file:`Examples/PointClouds/main.cpp`. See :ref:`Chap:Building` for how to compile
it with CMake, GNU Make, or a direct compiler invocation.

.. code-block:: bash

   cd Examples/PointClouds
   ./PointClouds.ex

The code that runs on either side
---------------------------------

Two query functors, ``ClosestPoint`` and ``NearestNeighbors``, each hold a cloud by value and have an
``EBGEOMETRY_HOST_DEVICE`` call operator: one maps a point to its closest point in the cloud, the
other maps a point's cloud index to its ``k`` nearest other points. A cloud owns no memory -- it is a
descriptor of the points and the search structure in a ``Pool`` -- so it is trivially copyable, and a
kernel can receive it by value once it describes memory the device can read. Both functors are
templates on the cloud type, so they serve either class.

Building the cloud
------------------

``run<Cloud>()`` does everything for one class. One constructor groups the points and builds the
search structure, storing both in a pool that must outlive the cloud and every copy of it:

.. code-block:: c++

   EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
   const Cloud      cloud(pool, positions);

``Cloud`` is ``PointCloudBVH<T>`` -- a tree that adapts to clustered or multi-scale clouds -- or
``PointCloudHashGrid<T>`` -- a uniform grid that builds faster on near-uniform ones. Switching between
them is a change of this one type.

Using it on the host
--------------------

``closestPoint(q)`` returns the nearest point to any point ``q``, and ``closestPoints(q, k, out)``
writes the ``k`` nearest to ``out``, nearest first. ``nearestNeighbor(i)`` returns the nearest *other*
point to point ``i`` of the cloud, and ``allNearestNeighbors(k)`` computes the whole graph in one
batched call (entry ``i * k + j`` is point ``i``'s ``j``-th nearest neighbor). Every search reports a
`PointCloud::Hit <doxygen/html/structEBGeometry_1_1PointCloud_1_1Hit.html>`__: a cloud index (the
point's position in ``positions``) and a squared distance.

A cloud carries no user data. Per-point data lives in an array of your own, indexed by the cloud
index; the example labels each point with its octant of the unit cube and prints the label of every
point it finds:

.. code-block:: c++

   const Hit hit = cloud.closestPoint(q);
   const int label = octant[hit.index];

The same queries on a GPU
-------------------------

The last part of ``run()`` freezes the pool, mirrors it in one piece into ``GPU::memoryResource()``
(device memory in a CUDA or HIP build), and rebases the cloud onto the copy. ``GPU::transform()``
then runs ``ClosestPoint`` once per query point and ``NearestNeighbors`` once per cloud index:
``allNearestNeighbors()`` is a host-only batch, so on a device the graph is computed one point per
thread. In a plain C++ build the same lines run on the host. See :ref:`Sec:ExamplesGPU`.
