.. _Chap:ExampleClosestPointBVH:

ClosestPointBVH
===============

Closest-point search over a point cloud using the turnkey ``PointCloudBVH`` class (see
:ref:`Chap:ImplemPointCloud`), which hides the whole point-cloud search pipeline -- grouping points
into SIMD-friendly leaves, building the ``PackedBVH``, and running the pruned traversal -- behind a
single constructor and a couple of query methods (``closestPoint()`` for the nearest point,
``closestPoints()`` for the ``k`` nearest, ascending by distance). This is the *external* query form:
the query points are arbitrary, not members of the cloud, unlike its neighbor-graph counterpart
:ref:`Chap:ExampleNearestNeighborBVH`.

The source for this example is at :file:`Examples/ClosestPointBVH/main.cpp`. See :ref:`Chap:Building`
for how to compile it with CMake, GNU Make, or a direct compiler invocation.

.. code-block:: bash

   cd Examples/ClosestPointBVH
   ./ClosestPointBVH.ex
