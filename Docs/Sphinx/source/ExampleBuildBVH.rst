.. _Chap:ExampleBuildBVH:

BuildBVH
========

Benchmarks EBGeometry's BVH construction strategies. It builds a BVH over a random point cloud with
each available strategy -- top-down (centroid, SAH, and midpoint partitioners), bottom-up along the
Morton, Nested, and Hilbert space-filling curves, and ClusterSAH -- and reports the build time for
each (see :ref:`Chap:BVHConstruction`). For most strategies it times two ways of reaching a
queryable ``PackedBVH``: the traditional ``TreeBVH``-then-``pack()`` path, and ``PackedBVH``'s direct
constructor, which packs the primitives straight into the flat, queryable layout without building a
``TreeBVH`` first (see :ref:`Chap:DirectSFCBuild`). It times *build* only, not query performance or
tree quality.

The source for this example is at :file:`Examples/BuildBVH/main.cpp`. See :ref:`Chap:Building`
for how to compile it with CMake, GNU Make, or a direct compiler invocation.

.. code-block:: bash

   cd Examples/BuildBVH
   ./BuildBVH.ex
