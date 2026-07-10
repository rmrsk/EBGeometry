.. _Chap:ExampleNestedBVH:

NestedBVH
=========

Builds a *nested* bounding volume hierarchy: an outer, BVH-accelerated CSG union whose primitives
are themselves BVH-backed mesh signed distance functions. One triangle mesh is loaded once into a
``TriMeshSDF`` (which owns an inner ``PackedBVH`` over its triangle groups) and then instanced at
several positions -- each placement a ``Translate`` wrapper sharing a pointer to that same
``TriMeshSDF`` -- and the placements are combined with ``BVHUnion``, which builds the outer
``PackedBVH`` over them. A single distance query therefore descends two levels of BVH — the outer
union hierarchy to locate the nearby placement, then the mesh's own inner hierarchy to find the
nearest triangle (see :ref:`Chap:BVH` and :ref:`Chap:ImplemCSG`).

The outer union shares each placement by pointer (the default ``BVH::SharedPtrStorage``) rather than
copying it, so the one inner mesh BVH is built and stored just once — the recommended way to nest
BVHs. See the *Storage policy* section of
:ref:`Chap:ImplemBVH` for why the outer level must not use ``BVH::ValueStorage`` here.

The source for this example is at :file:`Examples/NestedBVH/main.cpp`. See :ref:`Chap:Building`
for how to compile it with CMake, GNU Make, or a direct compiler invocation. Unlike the mesh-reading
examples above, it defaults to the ``dodecahedron.stl`` fixture shipped in the repository, so it
needs no submodule.

.. code-block:: bash

   cd Examples/NestedBVH
   ./NestedBVH.ex                       # defaults to ../../Tests/data/dodecahedron.stl
   ./NestedBVH.ex path/to/mesh.stl      # place copies of your own triangle mesh
