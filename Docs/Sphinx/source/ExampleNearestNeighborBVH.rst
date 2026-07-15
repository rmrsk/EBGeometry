.. _Chap:ExampleNearestNeighborBVH:

NearestNeighborBVH
==================

Nearest-neighbor search over a point cloud using the turnkey ``PointCloudBVH`` class (see
:ref:`Chap:ImplemPointCloud`). Where :ref:`Chap:ExampleClosestPointBVH` queries with arbitrary
*external* points, this example queries with points that are already in the cloud -- the classic
k-nearest-neighbor graph. ``allNearestNeighbors(k)`` computes the ``k`` nearest neighbors of every
point in one batched pass; a self query excludes the point itself, so the neighbors returned are the
nearest *other* points.

The source for this example is at :file:`Examples/NearestNeighborBVH/main.cpp`. See
:ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler invocation.

.. code-block:: bash

   cd Examples/NearestNeighborBVH
   ./NearestNeighborBVH.ex
