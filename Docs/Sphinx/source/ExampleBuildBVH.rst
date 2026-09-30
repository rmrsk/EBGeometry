.. _Chap:ExampleBuildBVH:

BuildBVH
========

Benchmarks EBGeometry's BVH construction strategies. For every ``BVH::Strategy`` -- ``SAH``,
``Centroid``, ``Midpoint``, ``ClusterSAH``, and ``SpaceFillingCurve`` along each of the Morton,
Nested, and Hilbert curves -- it builds a BVH over the same random point cloud and times three
things: ``BVH::buildTopology()`` alone (the tree's shape, before any primitive is stored), the full
``PackedBVH`` construction (the shape plus the primitives copied into leaf order), and a fixed
closest-point query workload on the result, run through ``PackedBVH::pruneTraverse()``. The first two
measure what each strategy costs to build; the third, what the tree it produces is worth to a query,
which is the other half of the trade-off (see :ref:`Chap:BVHConstruction`). It also reports each
tree's depth and node count, and checks that every strategy finds the same closest points.

The source for this example is at :file:`Examples/BuildBVH/main.cpp`. See :ref:`Chap:Building`
for how to compile it with CMake, GNU Make, or a direct compiler invocation.

.. code-block:: bash

   cd Examples/BuildBVH
   ./BuildBVH.ex
