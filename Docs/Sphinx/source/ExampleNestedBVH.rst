.. _Chap:ExampleNestedBVH:

NestedBVH
=========

Builds a *nested* bounding volume hierarchy: an outer, BVH-accelerated union whose primitives are
themselves BVH-backed mesh signed distance functions. The triangles of one mesh are read once, a
translated copy is built into a ``TriMeshSDF`` (which owns an inner ``PackedBVH`` over its triangle
groups) for each of several positions, and the placements are combined with ``BVHUnion``, which
builds the outer ``PackedBVH`` over them. A single distance query therefore descends two levels of
BVH -- the outer union hierarchy to locate the nearby placement, then that mesh's own inner
hierarchy to find the nearest triangle (see :ref:`Chap:BVH` and :ref:`Sec:BVHUnions`).

The outer union stores the ``TriMeshSDF``\ s by value, in the same ``Pool`` as their inner BVHs, so
the whole hierarchy is one plain value that can be mirrored to a GPU and evaluated there. Each
placement has its own copy of the triangles; sharing one mesh between translated placements is a
composition that returns with the tape. The program checks the nested union against the smallest
distance over every placement at random points, which must agree up to rounding.

The source for this example is at :file:`Examples/NestedBVH/main.cpp`. See :ref:`Chap:Building`
for how to compile it with CMake, GNU Make, or a direct compiler invocation. Unlike the mesh-reading
examples above, it defaults to the ``dodecahedron.stl`` fixture shipped in the repository, so it
needs no submodule.

.. code-block:: bash

   cd Examples/NestedBVH
   ./NestedBVH.ex                       # defaults to ../../Tests/data/dodecahedron.stl
   ./NestedBVH.ex path/to/mesh.stl      # place copies of your own triangle mesh
