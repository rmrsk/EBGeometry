.. _Chap:ExampleNearestNeighborHashGrid:

NearestNeighborHashGrid
=======================

Nearest-neighbor search over a point cloud using the turnkey ``PointCloudHashGrid`` class -- the
uniform-grid counterpart to :ref:`Chap:ExampleNearestNeighborBVH`, with the same interface, so
switching between the tree and the grid is a one-line type change (see :ref:`Chap:ImplemPointCloud`).
``allNearestNeighbors(k)`` computes the ``k`` nearest neighbors of every point in one batched pass; a
self query excludes the point itself, so the neighbors returned are the nearest *other* points. The
grid suits *near-uniform* clouds; for clustered or multi-scale clouds the density-adaptive
``PointCloudBVH`` is the better choice.

The source for this example is at :file:`Examples/NearestNeighborHashGrid/main.cpp`. See
:ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler invocation.

.. code-block:: bash

   cd Examples/NearestNeighborHashGrid
   ./NearestNeighborHashGrid.ex
